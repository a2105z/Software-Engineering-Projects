#pragma once

#include "../phase1_core/sequence.hpp"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>

namespace helixforge {

/**
 * Result of a scoring evaluation
 * Contains the metric name, value, and optional metadata
 */
struct ScoreResult {
    std::string metric_name;
    double value;
    std::unordered_map<std::string, double> metadata;
    std::string description;
    
    ScoreResult(std::string name, double val, std::string desc = "")
        : metric_name(std::move(name)), value(val), description(std::move(desc)) {}
};

/**
 * Collection of scoring results for a sequence
 */
class ScoreReport {
public:
    ScoreReport() = default;
    
    void add_score(const ScoreResult& result);
    void add_score(std::string name, double value, std::string description = "");
    
    const std::vector<ScoreResult>& results() const { return results_; }
    
    // Get specific score by name
    double get_score(const std::string& metric_name) const;
    bool has_score(const std::string& metric_name) const;
    
    // Aggregate scores
    double total_score() const;
    double weighted_score(const std::unordered_map<std::string, double>& weights) const;
    
    // Export to string representation
    std::string to_string() const;
    std::string to_json() const;
    
private:
    std::vector<ScoreResult> results_;
    std::unordered_map<std::string, size_t> name_to_index_;
};

/**
 * Abstract base class for all scoring metrics
 */
class ScoreMetric {
public:
    virtual ~ScoreMetric() = default;
    
    // Calculate score for a sequence
    virtual ScoreResult evaluate(const Sequence& seq) const = 0;
    
    // Get metric name
    virtual std::string name() const = 0;
    
    // Get metric description
    virtual std::string description() const = 0;
    
    // Check if metric is applicable to sequence type
    virtual bool is_applicable(SequenceType type) const = 0;
};

/**
 * Scoring engine - coordinates evaluation of multiple metrics
 */
class ScoringEngine {
public:
    ScoringEngine() = default;
    
    // Register a metric
    void register_metric(std::shared_ptr<ScoreMetric> metric);
    void register_metric(const std::string& metric_name);
    
    // Evaluate sequence with all registered metrics
    ScoreReport evaluate(const Sequence& seq) const;
    
    // Evaluate with specific metrics only
    ScoreReport evaluate(const Sequence& seq, const std::vector<std::string>& metric_names) const;
    
    // Get list of available metrics
    std::vector<std::string> available_metrics() const;
    
    // Clear all registered metrics
    void clear_metrics();
    
private:
    std::vector<std::shared_ptr<ScoreMetric>> metrics_;
    std::unordered_map<std::string, std::shared_ptr<ScoreMetric>> metric_map_;
    
    // Factory for built-in metrics
    static std::shared_ptr<ScoreMetric> create_builtin_metric(const std::string& name);
};

// Built-in metric names
namespace metrics {
    constexpr const char* GC_CONTENT = "gc_content";
    constexpr const char* CODON_ADAPTATION_INDEX = "codon_adaptation_index";
    constexpr const char* CODON_USAGE_BIAS = "codon_usage_bias";
    constexpr const char* ORF_INTEGRITY = "orf_integrity";
    constexpr const char* RESTRICTION_SITES = "restriction_sites";
    constexpr const char* SEQUENCE_COMPLEXITY = "sequence_complexity";
    constexpr const char* HOMOPOLYMER_RUNS = "homopolymer_runs";
    constexpr const char* DINUCLEOTIDE_ODDS = "dinucleotide_odds";
    constexpr const char* RARE_CODONS = "rare_codons";
    constexpr const char* SECONDARY_STRUCTURE = "secondary_structure";
}

} // namespace helixforge
