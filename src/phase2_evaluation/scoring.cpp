#include "scoring.hpp"
#include "metrics.hpp"
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <algorithm>

namespace helixforge {

// ============================================================================
// ScoreReport Implementation
// ============================================================================

void ScoreReport::add_score(const ScoreResult& result) {
    name_to_index_[result.metric_name] = results_.size();
    results_.push_back(result);
}

void ScoreReport::add_score(std::string name, double value, std::string description) {
    add_score(ScoreResult(std::move(name), value, std::move(description)));
}

double ScoreReport::get_score(const std::string& metric_name) const {
    auto it = name_to_index_.find(metric_name);
    if (it == name_to_index_.end()) {
        throw std::out_of_range("Metric not found: " + metric_name);
    }
    return results_[it->second].value;
}

bool ScoreReport::has_score(const std::string& metric_name) const {
    return name_to_index_.find(metric_name) != name_to_index_.end();
}

double ScoreReport::total_score() const {
    double total = 0.0;
    for (const auto& result : results_) {
        total += result.value;
    }
    return total;
}

double ScoreReport::weighted_score(const std::unordered_map<std::string, double>& weights) const {
    double total = 0.0;
    for (const auto& result : results_) {
        auto it = weights.find(result.metric_name);
        if (it != weights.end()) {
            total += result.value * it->second;
        }
    }
    return total;
}

std::string ScoreReport::to_string() const {
    std::ostringstream oss;
    oss << "Score Report:\n";
    oss << "-------------\n";
    
    for (const auto& result : results_) {
        oss << std::setw(30) << std::left << result.metric_name << ": "
            << std::setw(10) << std::right << std::fixed << std::setprecision(4) 
            << result.value;
        
        if (!result.description.empty()) {
            oss << " (" << result.description << ")";
        }
        
        oss << "\n";
        
        if (!result.metadata.empty()) {
            for (const auto& [key, val] : result.metadata) {
                oss << "  " << std::setw(28) << std::left << key << ": "
                    << std::setw(10) << std::right << std::fixed << std::setprecision(4) 
                    << val << "\n";
            }
        }
    }
    
    return oss.str();
}

std::string ScoreReport::to_json() const {
    std::ostringstream oss;
    oss << "{\n  \"scores\": [\n";
    
    for (size_t i = 0; i < results_.size(); ++i) {
        const auto& result = results_[i];
        oss << "    {\n";
        oss << "      \"metric\": \"" << result.metric_name << "\",\n";
        oss << "      \"value\": " << std::fixed << std::setprecision(6) << result.value;
        
        if (!result.description.empty()) {
            oss << ",\n      \"description\": \"" << result.description << "\"";
        }
        
        if (!result.metadata.empty()) {
            oss << ",\n      \"metadata\": {\n";
            size_t j = 0;
            for (const auto& [key, val] : result.metadata) {
                oss << "        \"" << key << "\": " << std::fixed << std::setprecision(6) << val;
                if (++j < result.metadata.size()) oss << ",";
                oss << "\n";
            }
            oss << "      }";
        }
        
        oss << "\n    }";
        if (i < results_.size() - 1) oss << ",";
        oss << "\n";
    }
    
    oss << "  ],\n";
    oss << "  \"total_score\": " << std::fixed << std::setprecision(6) << total_score() << "\n";
    oss << "}";
    
    return oss.str();
}

// ============================================================================
// ScoringEngine Implementation
// ============================================================================

void ScoringEngine::register_metric(std::shared_ptr<ScoreMetric> metric) {
    metrics_.push_back(metric);
    metric_map_[metric->name()] = metric;
}

void ScoringEngine::register_metric(const std::string& metric_name) {
    auto metric = create_builtin_metric(metric_name);
    if (metric) {
        register_metric(metric);
    } else {
        throw std::invalid_argument("Unknown metric: " + metric_name);
    }
}

ScoreReport ScoringEngine::evaluate(const Sequence& seq) const {
    ScoreReport report;
    
    for (const auto& metric : metrics_) {
        if (metric->is_applicable(seq.type())) {
            try {
                ScoreResult result = metric->evaluate(seq);
                report.add_score(result);
            } catch (const std::exception& e) {
                // Add error as zero score with description
                report.add_score(metric->name(), 0.0, std::string("Error: ") + e.what());
            }
        }
    }
    
    return report;
}

ScoreReport ScoringEngine::evaluate(const Sequence& seq, const std::vector<std::string>& metric_names) const {
    ScoreReport report;
    
    for (const auto& name : metric_names) {
        auto it = metric_map_.find(name);
        if (it != metric_map_.end()) {
            const auto& metric = it->second;
            if (metric->is_applicable(seq.type())) {
                try {
                    ScoreResult result = metric->evaluate(seq);
                    report.add_score(result);
                } catch (const std::exception& e) {
                    report.add_score(metric->name(), 0.0, std::string("Error: ") + e.what());
                }
            }
        } else {
            throw std::invalid_argument("Metric not found: " + name);
        }
    }
    
    return report;
}

std::vector<std::string> ScoringEngine::available_metrics() const {
    std::vector<std::string> names;
    names.reserve(metrics_.size());
    for (const auto& metric : metrics_) {
        names.push_back(metric->name());
    }
    return names;
}

void ScoringEngine::clear_metrics() {
    metrics_.clear();
    metric_map_.clear();
}

std::shared_ptr<ScoreMetric> ScoringEngine::create_builtin_metric(const std::string& name) {
    if (name == metrics::GC_CONTENT) {
        return std::make_shared<GCContentMetric>();
    } else if (name == metrics::CODON_ADAPTATION_INDEX) {
        return std::make_shared<CodonAdaptationIndexMetric>();
    } else if (name == metrics::CODON_USAGE_BIAS) {
        return std::make_shared<CodonUsageBiasMetric>();
    } else if (name == metrics::ORF_INTEGRITY) {
        return std::make_shared<ORFIntegrityMetric>();
    } else if (name == metrics::RESTRICTION_SITES) {
        return std::make_shared<RestrictionSitesMetric>();
    } else if (name == metrics::SEQUENCE_COMPLEXITY) {
        return std::make_shared<SequenceComplexityMetric>();
    } else if (name == metrics::HOMOPOLYMER_RUNS) {
        return std::make_shared<HomopolymerRunsMetric>();
    } else if (name == metrics::DINUCLEOTIDE_ODDS) {
        return std::make_shared<DinucleotideOddsMetric>();
    } else if (name == metrics::RARE_CODONS) {
        return std::make_shared<RareCodensMetric>();
    } else if (name == metrics::SECONDARY_STRUCTURE) {
        return std::make_shared<SecondaryStructureMetric>();
    }
    
    return nullptr;
}

} // namespace helixforge
