#pragma once

#include "../phase1_core/sequence.hpp"
#include "../phase2_evaluation/scoring.hpp"
#include "../phase2_evaluation/constraint_engine.hpp"
#include "../phase3_mutation/mutation_engine.hpp"
#include "../phase4_optimization/optimizer.hpp"
#include <string>
#include <vector>
#include <ostream>

namespace helixforge {

/**
 * JSON output utilities for HelixForge
 * Provides structured, machine-readable output
 */
class JSONOutput {
public:
    // Escape string for JSON
    static std::string escape_json(const std::string& str);
    
    // Format sequence as JSON
    static std::string sequence_to_json(const Sequence& seq, bool include_data = true);
    
    // Format score report as JSON
    static std::string score_report_to_json(const ScoreReport& report);
    
    // Format constraint report as JSON
    static std::string constraint_report_to_json(const ConstraintReport& report);
    
    // Format mutation result as JSON
    static std::string mutation_result_to_json(const MutationResult& result);
    
    // Format optimization result as JSON
    static std::string optimization_result_to_json(
        const OptimizationResult& result,
        bool include_history = false
    );
    
    // Format multiple sequences as JSON array
    static std::string sequences_to_json(const std::vector<Sequence>& sequences);
    
    // Create complete evaluation report
    static std::string create_evaluation_report(
        const Sequence& seq,
        const ScoreReport& scores,
        const ConstraintReport* constraints = nullptr
    );
    
    // Create complete optimization report
    static std::string create_optimization_report(
        const Sequence& initial,
        const OptimizationResult& result,
        const std::string& objective_name,
        const std::string& algorithm_name
    );
    
    // Write JSON to file
    static bool write_to_file(const std::string& json, const std::string& filepath);
    
    // Pretty print JSON (with indentation)
    static std::string pretty_print(const std::string& json, size_t indent_size = 2);
};

} // namespace helixforge
