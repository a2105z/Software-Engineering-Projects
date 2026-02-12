#include "optimizer.hpp"
#include "hill_climbing.hpp"
#include "simulated_annealing.hpp"
#include "beam_search.hpp"
#include "genetic_algorithm.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>

namespace helixforge {

// ============================================================================
// OptimizationResult Implementation
// ============================================================================

void OptimizationResult::update_best(const Sequence& seq, double score) {
    if (score > best_score) {
        best_sequence = seq;
        best_score = score;
    }
}

std::string OptimizationResult::to_string() const {
    std::ostringstream oss;
    oss << "Optimization Result:\n";
    oss << "-------------------\n";
    oss << "Best Score: " << std::fixed << std::setprecision(6) << best_score << "\n";
    oss << "Iterations: " << iterations << "\n";
    oss << "Evaluations: " << evaluations << "\n";
    oss << "Converged: " << (converged ? "Yes" : "No") << "\n";
    if (!termination_reason.empty()) {
        oss << "Termination: " << termination_reason << "\n";
    }
    oss << "Best Sequence Length: " << best_sequence.length() << "\n";
    return oss.str();
}

std::string OptimizationResult::to_json() const {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"best_score\": " << std::fixed << std::setprecision(6) << best_score << ",\n";
    oss << "  \"iterations\": " << iterations << ",\n";
    oss << "  \"evaluations\": " << evaluations << ",\n";
    oss << "  \"converged\": " << (converged ? "true" : "false") << ",\n";
    oss << "  \"termination_reason\": \"" << termination_reason << "\",\n";
    oss << "  \"sequence_length\": " << best_sequence.length() << ",\n";
    oss << "  \"score_history\": [";
    
    for (size_t i = 0; i < score_history.size(); ++i) {
        oss << std::fixed << std::setprecision(6) << score_history[i];
        if (i < score_history.size() - 1) oss << ", ";
    }
    oss << "]\n";
    oss << "}";
    
    return oss.str();
}

// ============================================================================
// OptimizationConfig Implementation
// ============================================================================

void OptimizationConfig::validate() const {
    if (max_iterations == 0 && max_evaluations == 0) {
        throw std::invalid_argument("Must specify at least one termination criterion");
    }
    if (min_improvement < 0) {
        throw std::invalid_argument("min_improvement must be non-negative");
    }
}

// ============================================================================
// Optimizer Base Implementation
// ============================================================================

double Optimizer::evaluate(const Sequence& seq, ObjectiveFunction& objective) {
    return objective(seq);
}

bool Optimizer::satisfies_constraints(const Sequence& seq, const ConstraintEngine* constraints) {
    if (!constraints) return true;
    return constraints->satisfies_all(seq);
}

bool Optimizer::should_terminate(const OptimizationResult& result) const {
    if (result.iterations >= config_.max_iterations) return true;
    if (result.evaluations >= config_.max_evaluations) return true;
    if (result.best_score >= config_.target_score) return true;
    return false;
}

void Optimizer::log_progress(const OptimizationResult& result) const {
    if (!config_.verbose) return;
    if (result.iterations % config_.log_frequency != 0) return;
    
    std::cout << "[Iter " << result.iterations 
              << "] Score: " << std::fixed << std::setprecision(4) << result.best_score
              << " (Evals: " << result.evaluations << ")" << std::endl;
}

// ============================================================================
// ObjectiveFactory Implementation
// ============================================================================

ObjectiveFunction ObjectiveFactory::create(ObjectiveType type, double parameter) {
    switch (type) {
        case ObjectiveType::MAXIMIZE_GC:
            return maximize_gc();
        case ObjectiveType::MINIMIZE_GC:
            return minimize_gc();
        case ObjectiveType::TARGET_GC:
            return target_gc(parameter);
        case ObjectiveType::MAXIMIZE_CAI:
            return maximize_cai();
        case ObjectiveType::MINIMIZE_RARE_CODONS:
            return minimize_rare_codons();
        case ObjectiveType::MAXIMIZE_EXPRESSION:
            return maximize_expression();
        case ObjectiveType::MINIMIZE_STRUCTURE:
            return minimize_structure();
        default:
            throw std::invalid_argument("Unknown objective type");
    }
}

ObjectiveFunction ObjectiveFactory::maximize_gc() {
    return [](const Sequence& seq) {
        return seq.gc_content();
    };
}

ObjectiveFunction ObjectiveFactory::minimize_gc() {
    return [](const Sequence& seq) {
        return 1.0 - seq.gc_content();
    };
}

ObjectiveFunction ObjectiveFactory::target_gc(double target) {
    return [target](const Sequence& seq) {
        double gc = seq.gc_content();
        return 1.0 - std::abs(gc - target);
    };
}

ObjectiveFunction ObjectiveFactory::maximize_cai() {
    return [](const Sequence& seq) {
        ScoringEngine engine;
        engine.register_metric(metrics::CODON_ADAPTATION_INDEX);
        auto report = engine.evaluate(seq);
        return report.has_score(metrics::CODON_ADAPTATION_INDEX) ?
            report.get_score(metrics::CODON_ADAPTATION_INDEX) : 0.0;
    };
}

ObjectiveFunction ObjectiveFactory::minimize_rare_codons() {
    return [](const Sequence& seq) {
        ScoringEngine engine;
        engine.register_metric(metrics::RARE_CODONS);
        auto report = engine.evaluate(seq);
        if (report.has_score(metrics::RARE_CODONS)) {
            double rare_pct = report.get_score(metrics::RARE_CODONS);
            return 1.0 - (rare_pct / 100.0);  // Convert to 0-1 and invert
        }
        return 0.0;
    };
}

ObjectiveFunction ObjectiveFactory::maximize_expression() {
    return [](const Sequence& seq) {
        ScoringEngine engine;
        engine.register_metric(metrics::CODON_ADAPTATION_INDEX);
        engine.register_metric(metrics::RARE_CODONS);
        
        auto report = engine.evaluate(seq);
        
        double cai = report.has_score(metrics::CODON_ADAPTATION_INDEX) ?
            report.get_score(metrics::CODON_ADAPTATION_INDEX) : 0.0;
        
        double rare_pct = report.has_score(metrics::RARE_CODONS) ?
            report.get_score(metrics::RARE_CODONS) : 0.0;
        
        double rare_score = 1.0 - (rare_pct / 100.0);
        
        // Weighted combination
        return 0.6 * cai + 0.4 * rare_score;
    };
}

ObjectiveFunction ObjectiveFactory::minimize_structure() {
    return [](const Sequence& seq) {
        ScoringEngine engine;
        engine.register_metric(metrics::SECONDARY_STRUCTURE);
        auto report = engine.evaluate(seq);
        if (report.has_score(metrics::SECONDARY_STRUCTURE)) {
            double energy = report.get_score(metrics::SECONDARY_STRUCTURE);
            // More negative energy = more stable structure (bad for minimizing)
            // Convert to 0-1 score where 1 = minimal structure
            return 1.0 / (1.0 + std::abs(energy));
        }
        return 0.0;
    };
}

ObjectiveFunction ObjectiveFactory::weighted_sum(
    const std::vector<ObjectiveFunction>& objectives,
    const std::vector<double>& weights)
{
    if (objectives.size() != weights.size()) {
        throw std::invalid_argument("objectives and weights must have same size");
    }
    
    return [objectives, weights](const Sequence& seq) {
        double total = 0.0;
        for (size_t i = 0; i < objectives.size(); ++i) {
            total += weights[i] * objectives[i](seq);
        }
        return total;
    };
}

// ============================================================================
// OptimizerFactory Implementation
// ============================================================================

std::unique_ptr<Optimizer> OptimizerFactory::create(OptimizerType type, uint64_t seed) {
    switch (type) {
        case OptimizerType::HILL_CLIMBING:
            return std::make_unique<HillClimbing>(seed);
        case OptimizerType::SIMULATED_ANNEALING:
            return std::make_unique<SimulatedAnnealing>(seed);
        case OptimizerType::BEAM_SEARCH:
            return std::make_unique<BeamSearch>(seed);
        case OptimizerType::GENETIC_ALGORITHM:
            return std::make_unique<GeneticAlgorithm>(seed);
        default:
            throw std::invalid_argument("Unknown optimizer type");
    }
}

} // namespace helixforge
