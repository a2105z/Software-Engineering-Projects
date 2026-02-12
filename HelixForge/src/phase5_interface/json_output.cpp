#include "json_output.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>

namespace helixforge {

std::string JSONOutput::escape_json(const std::string& str) {
    std::ostringstream oss;
    for (char c : str) {
        switch (c) {
            case '"': oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default: oss << c; break;
        }
    }
    return oss.str();
}

std::string JSONOutput::sequence_to_json(const Sequence& seq, bool include_data) {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"id\": \"" << escape_json(seq.id()) << "\",\n";
    oss << "  \"type\": \"" << sequence_type_to_string(seq.type()) << "\",\n";
    oss << "  \"length\": " << seq.length();
    
    if (include_data) {
        oss << ",\n  \"data\": \"" << escape_json(seq.data()) << "\"";
    }
    
    if (!seq.description().empty()) {
        oss << ",\n  \"description\": \"" << escape_json(seq.description()) << "\"";
    }
    
    oss << "\n}";
    return oss.str();
}

std::string JSONOutput::score_report_to_json(const ScoreReport& report) {
    return report.to_json();
}

std::string JSONOutput::constraint_report_to_json(const ConstraintReport& report) {
    return report.to_json();
}

std::string JSONOutput::mutation_result_to_json(const MutationResult& result) {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"mutated_sequence\": " << sequence_to_json(result.mutated_sequence) << ",\n";
    oss << "  \"mutation_count\": " << result.mutations.size() << ",\n";
    oss << "  \"constraint_satisfied\": " << (result.constraint_satisfied ? "true" : "false");
    
    if (!result.mutations.empty()) {
        oss << ",\n  \"mutations\": [\n";
        for (size_t i = 0; i < result.mutations.size(); ++i) {
            oss << "    {\"type\": \"" << "mutation" << "\", "
                << "\"position\": " << result.mutations[i].position << "}";
            if (i < result.mutations.size() - 1) oss << ",";
            oss << "\n";
        }
        oss << "  ]";
    }
    
    oss << "\n}";
    return oss.str();
}

std::string JSONOutput::optimization_result_to_json(
    const OptimizationResult& result,
    bool include_history)
{
    return result.to_json();
}

std::string JSONOutput::sequences_to_json(const std::vector<Sequence>& sequences) {
    std::ostringstream oss;
    oss << "[\n";
    for (size_t i = 0; i < sequences.size(); ++i) {
        oss << "  " << sequence_to_json(sequences[i], false);
        if (i < sequences.size() - 1) oss << ",";
        oss << "\n";
    }
    oss << "]";
    return oss.str();
}

std::string JSONOutput::create_evaluation_report(
    const Sequence& seq,
    const ScoreReport& scores,
    const ConstraintReport* constraints)
{
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"sequence\": " << sequence_to_json(seq, false) << ",\n";
    oss << "  \"scores\": " << score_report_to_json(scores);
    
    if (constraints) {
        oss << ",\n  \"constraints\": " << constraint_report_to_json(*constraints);
    }
    
    oss << "\n}";
    return oss.str();
}

std::string JSONOutput::create_optimization_report(
    const Sequence& initial,
    const OptimizationResult& result,
    const std::string& objective_name,
    const std::string& algorithm_name)
{
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"algorithm\": \"" << algorithm_name << "\",\n";
    oss << "  \"objective\": \"" << objective_name << "\",\n";
    oss << "  \"initial_sequence\": " << sequence_to_json(initial, false) << ",\n";
    oss << "  \"result\": " << optimization_result_to_json(result, true) << "\n";
    oss << "}";
    return oss.str();
}

bool JSONOutput::write_to_file(const std::string& json, const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file) return false;
    file << json;
    return true;
}

std::string JSONOutput::pretty_print(const std::string& json, size_t indent_size) {
    // Simple pretty print (just return as-is, already formatted)
    return json;
}

} // namespace helixforge
