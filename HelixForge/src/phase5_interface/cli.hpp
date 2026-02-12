#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <functional>

namespace helixforge {

/**
 * Command-line argument parser
 */
class ArgumentParser {
public:
    ArgumentParser(const std::string& program_name, const std::string& description);
    
    // Add positional argument
    void add_argument(const std::string& name, const std::string& help);
    
    // Add optional argument
    void add_option(const std::string& name, const std::string& short_name,
                   const std::string& help, const std::string& default_value = "");
    
    // Add flag (boolean option)
    void add_flag(const std::string& name, const std::string& short_name,
                 const std::string& help);
    
    // Parse arguments
    bool parse(int argc, char* argv[]);
    
    // Get values
    std::string get(const std::string& name) const;
    bool has(const std::string& name) const;
    bool get_flag(const std::string& name) const;
    std::vector<std::string> get_list(const std::string& name) const;
    
    // Help message
    std::string help() const;
    
private:
    std::string program_name_;
    std::string description_;
    
    struct Argument {
        std::string name;
        std::string short_name;
        std::string help;
        std::string default_value;
        bool is_flag;
        bool is_positional;
        bool required;
    };
    
    std::vector<Argument> arguments_;
    std::map<std::string, std::string> values_;
    std::map<std::string, bool> flags_;
    
    std::optional<Argument*> find_argument(const std::string& name);
    Argument* find_argument_safe(const std::string& name);
};

/**
 * CLI Application for HelixForge
 */
class CLIApp {
public:
    CLIApp();
    
    // Main entry point
    int run(int argc, char* argv[]);
    
private:
    // Command handlers
    int handle_evaluate(const ArgumentParser& args);
    int handle_mutate(const ArgumentParser& args);
    int handle_optimize(const ArgumentParser& args);
    int handle_scan(const ArgumentParser& args);
    
    // Utility functions
    void print_version() const;
    void print_usage() const;
    
    // Setup parsers for each command
    ArgumentParser create_evaluate_parser();
    ArgumentParser create_mutate_parser();
    ArgumentParser create_optimize_parser();
    ArgumentParser create_scan_parser();
};

} // namespace helixforge
