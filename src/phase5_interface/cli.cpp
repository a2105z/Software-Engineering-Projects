#include "cli.hpp"
#include "json_output.hpp"
#include "../phase1_core/sequence.hpp"
#include "../phase2_evaluation/scoring.hpp"
#include "../phase2_evaluation/constraint_engine.hpp"
#include "../phase3_mutation/mutation_engine.hpp"
#include "../phase4_optimization/optimizer.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace helixforge {

// ============================================================================
// ArgumentParser Implementation
// ============================================================================

ArgumentParser::ArgumentParser(const std::string& program_name, const std::string& description)
    : program_name_(program_name), description_(description) {}

void ArgumentParser::add_argument(const std::string& name, const std::string& help) {
    Argument arg;
    arg.name = name;
    arg.help = help;
    arg.is_flag = false;
    arg.is_positional = true;
    arg.required = true;
    arguments_.push_back(arg);
}

void ArgumentParser::add_option(const std::string& name, const std::string& short_name,
                               const std::string& help, const std::string& default_value) {
    Argument arg;
    arg.name = name;
    arg.short_name = short_name;
    arg.help = help;
    arg.default_value = default_value;
    arg.is_flag = false;
    arg.is_positional = false;
    arg.required = default_value.empty();
    arguments_.push_back(arg);
    
    if (!default_value.empty()) {
        values_[name] = default_value;
    }
}

void ArgumentParser::add_flag(const std::string& name, const std::string& short_name,
                             const std::string& help) {
    Argument arg;
    arg.name = name;
    arg.short_name = short_name;
    arg.help = help;
    arg.is_flag = true;
    arg.is_positional = false;
    arg.required = false;
    arguments_.push_back(arg);
    flags_[name] = false;
}

bool ArgumentParser::parse(int argc, char* argv[]) {
    std::vector<std::string> positional_values;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            std::cout << help() << std::endl;
            return false;
        }
        
        if (arg.substr(0, 2) == "--" || (arg.size() > 1 && arg[0] == '-' && arg[1] != '-')) {
            // Option or flag
            std::string key = arg.substr(0, 2) == "--" ? arg.substr(2) : arg.substr(1);
            
            // Find argument - use safe version
            auto* found_arg = find_argument_safe(key);
            if (!found_arg) {
                std::cerr << "Unknown option: " << arg << std::endl;
                return false;
            }
            
            if (found_arg->is_flag) {
                flags_[found_arg->name] = true;
            } else {
                if (i + 1 >= argc) {
                    std::cerr << "Option " << arg << " requires a value" << std::endl;
                    return false;
                }
                values_[found_arg->name] = argv[++i];
            }
        } else {
            // Positional argument
            positional_values.push_back(arg);
        }
    }
    
    // Assign positional values
    size_t pos_idx = 0;
    for (auto& arg : arguments_) {
        if (arg.is_positional && pos_idx < positional_values.size()) {
            values_[arg.name] = positional_values[pos_idx++];
        }
    }
    
    // Check required arguments
    for (const auto& arg : arguments_) {
        if (arg.required && values_.find(arg.name) == values_.end()) {
            std::cerr << "Missing required argument: " << arg.name << std::endl;
            return false;
        }
    }
    
    return true;
}

std::string ArgumentParser::get(const std::string& name) const {
    auto it = values_.find(name);
    return it != values_.end() ? it->second : "";
}

bool ArgumentParser::has(const std::string& name) const {
    return values_.find(name) != values_.end();
}

bool ArgumentParser::get_flag(const std::string& name) const {
    auto it = flags_.find(name);
    return it != flags_.end() ? it->second : false;
}

std::vector<std::string> ArgumentParser::get_list(const std::string& name) const {
    std::vector<std::string> result;
    std::string value = get(name);
    if (value.empty()) return result;
    
    std::istringstream ss(value);
    std::string item;
    while (std::getline(ss, item, ',')) {
        result.push_back(item);
    }
    return result;
}

std::string ArgumentParser::help() const {
    std::ostringstream oss;
    oss << program_name_ << " - " << description_ << "\n\n";
    oss << "Usage: " << program_name_;
    
    for (const auto& arg : arguments_) {
        if (arg.is_positional) {
            oss << " <" << arg.name << ">";
        }
    }
    
    oss << " [options]\n\n";
    
    if (!arguments_.empty()) {
        oss << "Arguments:\n";
        for (const auto& arg : arguments_) {
            if (arg.is_positional) {
                oss << "  " << arg.name << "\t" << arg.help << "\n";
            }
        }
        
        oss << "\nOptions:\n";
        for (const auto& arg : arguments_) {
            if (!arg.is_positional) {
                oss << "  --" << arg.name;
                if (!arg.short_name.empty()) {
                    oss << ", -" << arg.short_name;
                }
                oss << "\t" << arg.help;
                if (!arg.default_value.empty()) {
                    oss << " (default: " << arg.default_value << ")";
                }
                oss << "\n";
            }
        }
    }
    
    return oss.str();
}

std::optional<ArgumentParser::Argument*> ArgumentParser::find_argument(const std::string& name) {
    for (auto& arg : arguments_) {
        if (arg.name == name || arg.short_name == name) {
            return &arg;
        }
    }
    return std::nullopt;
}

// Helper to safely get argument
ArgumentParser::Argument* ArgumentParser::find_argument_safe(const std::string& name) {
    for (auto& arg : arguments_) {
        if (arg.name == name || arg.short_name == name) {
            return &arg;
        }
    }
    return nullptr;
}

// ============================================================================
// CLIApp Implementation
// ============================================================================

CLIApp::CLIApp() {}

int CLIApp::run(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }
    
    std::string command = argv[1];
    
    if (command == "--version" || command == "-v") {
        print_version();
        return 0;
    }
    
    if (command == "--help" || command == "-h") {
        print_usage();
        return 0;
    }
    
    // Shift arguments
    int new_argc = argc - 1;
    char** new_argv = argv + 1;
    
    if (command == "evaluate") {
        auto parser = create_evaluate_parser();
        if (!parser.parse(new_argc, new_argv)) return 1;
        return handle_evaluate(parser);
    } else if (command == "mutate") {
        auto parser = create_mutate_parser();
        if (!parser.parse(new_argc, new_argv)) return 1;
        return handle_mutate(parser);
    } else if (command == "optimize") {
        auto parser = create_optimize_parser();
        if (!parser.parse(new_argc, new_argv)) return 1;
        return handle_optimize(parser);
    } else if (command == "scan") {
        auto parser = create_scan_parser();
        if (!parser.parse(new_argc, new_argv)) return 1;
        return handle_scan(parser);
    } else {
        std::cerr << "Unknown command: " << command << std::endl;
        print_usage();
        return 1;
    }
}

void CLIApp::print_version() const {
    std::cout << "HelixForge v1.0.0\n";
    std::cout << "C++17 Biological Sequence Optimization Engine\n";
}

void CLIApp::print_usage() const {
    std::cout << "HelixForge - Biological Sequence Optimization Engine\n\n";
    std::cout << "Usage: helixforge <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  evaluate    Evaluate sequence metrics\n";
    std::cout << "  mutate      Generate sequence mutations\n";
    std::cout << "  optimize    Optimize sequence under constraints\n";
    std::cout << "  scan        Perform mutational scanning\n\n";
    std::cout << "Use 'helixforge <command> --help' for command-specific help\n";
}

ArgumentParser CLIApp::create_evaluate_parser() {
    ArgumentParser parser("helixforge evaluate", "Evaluate sequence metrics");
    parser.add_argument("input", "Input FASTA file");
    parser.add_option("metrics", "m", "Comma-separated list of metrics", "gc_content,codon_adaptation_index");
    parser.add_option("output", "o", "Output JSON file", "");
    parser.add_flag("verbose", "v", "Verbose output");
    return parser;
}

ArgumentParser CLIApp::create_mutate_parser() {
    ArgumentParser parser("helixforge mutate", "Generate sequence mutations");
    parser.add_argument("input", "Input FASTA file");
    parser.add_option("num", "n", "Number of mutations", "1");
    parser.add_option("output", "o", "Output FASTA file", "");
    parser.add_option("seed", "s", "Random seed", "42");
    return parser;
}

ArgumentParser CLIApp::create_optimize_parser() {
    ArgumentParser parser("helixforge optimize", "Optimize sequence");
    parser.add_argument("input", "Input FASTA file");
    parser.add_option("objective", "obj", "Objective function", "maximize_expression");
    parser.add_option("algorithm", "alg", "Algorithm (hill_climbing, anneal, beam, genetic)", "anneal");
    parser.add_option("constraints", "c", "Constraints (e.g., gc=0.45-0.60)", "");
    parser.add_option("iterations", "i", "Max iterations", "1000");
    parser.add_option("seed", "s", "Random seed", "42");
    parser.add_option("output", "o", "Output JSON file", "");
    return parser;
}

ArgumentParser CLIApp::create_scan_parser() {
    ArgumentParser parser("helixforge scan", "Mutational scanning");
    parser.add_argument("input", "Input FASTA file");
    parser.add_option("output", "o", "Output JSON file", "");
    return parser;
}

int CLIApp::handle_evaluate(const ArgumentParser& args) {
    try {
        // Read input
        std::string input_file = args.get("input");
        std::ifstream file(input_file);
        if (!file) {
            std::cerr << "Error: Cannot open file " << input_file << std::endl;
            return 1;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        auto seq = Sequence::from_fasta_single(buffer.str());
        
        // Setup scoring engine
        ScoringEngine engine;
        auto metric_names = args.get_list("metrics");
        for (const auto& name : metric_names) {
            engine.register_metric(name);
        }
        
        // Evaluate
        auto report = engine.evaluate(seq);
        
        // Output
        if (args.has("output")) {
            std::string json = JSONOutput::create_evaluation_report(seq, report);
            JSONOutput::write_to_file(json, args.get("output"));
        } else {
            std::cout << report.to_string();
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

int CLIApp::handle_mutate(const ArgumentParser& args) {
    try {
        std::string input_file = args.get("input");
        std::ifstream file(input_file);
        if (!file) {
            std::cerr << "Error: Cannot open file " << input_file << std::endl;
            return 1;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        auto seq = Sequence::from_fasta_single(buffer.str());
        
        // Create mutation engine
        uint64_t seed = 42;
        try {
            seed = std::stoull(args.get("seed"));
        } catch (const std::exception&) {
            std::cerr << "Warning: Invalid seed, using default (42)" << std::endl;
        }
        MutationEngine engine(seed);
        
        size_t num = 1;
        try {
            num = std::stoull(args.get("num"));
        } catch (const std::exception&) {
            std::cerr << "Error: Invalid number for mutations" << std::endl;
            return 1;
        }
        auto result = engine.apply_n_mutations(seq, num);
        
        // Output
        if (args.has("output")) {
            std::ofstream out(args.get("output"));
            out << result.mutated_sequence.to_fasta();
        } else {
            std::cout << result.mutated_sequence.to_fasta();
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

int CLIApp::handle_optimize(const ArgumentParser& args) {
    try {
        std::string input_file = args.get("input");
        std::ifstream file(input_file);
        if (!file) {
            std::cerr << "Error: Cannot open file " << input_file << std::endl;
            return 1;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        auto seq = Sequence::from_fasta_single(buffer.str());
        
        // Create optimizer
        uint64_t seed = 42;
        try {
            seed = std::stoull(args.get("seed"));
        } catch (const std::exception&) {
            std::cerr << "Warning: Invalid seed, using default (42)" << std::endl;
        }
        std::string alg = args.get("algorithm");
        OptimizerType alg_type = OptimizerType::SIMULATED_ANNEALING;
        if (alg == "hill_climbing") alg_type = OptimizerType::HILL_CLIMBING;
        else if (alg == "beam") alg_type = OptimizerType::BEAM_SEARCH;
        else if (alg == "genetic") alg_type = OptimizerType::GENETIC_ALGORITHM;
        
        auto optimizer = OptimizerFactory::create(alg_type, seed);
        
        // Configure
        OptimizationConfig config;
        try {
            config.max_iterations = std::stoull(args.get("iterations"));
        } catch (const std::exception&) {
            std::cerr << "Warning: Invalid iterations, using default (1000)" << std::endl;
            config.max_iterations = 1000;
        }
        config.verbose = true;
        optimizer->set_config(config);
        
        // Create objective
        std::string obj_name = args.get("objective");
        auto objective = ObjectiveFactory::create(ObjectiveType::MAXIMIZE_EXPRESSION);
        
        // Optimize
        auto result = optimizer->optimize(seq, objective);
        
        // Output
        std::string json = JSONOutput::create_optimization_report(seq, result, obj_name, alg);
        if (args.has("output")) {
            JSONOutput::write_to_file(json, args.get("output"));
        } else {
            std::cout << JSONOutput::pretty_print(json) << std::endl;
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

int CLIApp::handle_scan(const ArgumentParser& args) {
    std::cerr << "Mutational scanning not yet implemented\n";
    return 1;
}

} // namespace helixforge
