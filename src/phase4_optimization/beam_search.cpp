#include "beam_search.hpp"
#include <algorithm>

namespace helixforge {

BeamSearch::BeamSearch(uint64_t seed)
    : rng_(std::make_unique<RNG>(seed)) {
    
    mutation_engine_ = std::make_shared<MutationEngine>(*rng_);
}

OptimizationResult BeamSearch::optimize(
    const Sequence& initial_sequence,
    ObjectiveFunction objective,
    const ConstraintEngine* constraints)
{
    OptimizationResult result;
    
    // Validate initial sequence
    if (!satisfies_constraints(initial_sequence, constraints)) {
        result.converged = false;
        result.termination_reason = "Initial sequence violates constraints";
        return result;
    }
    
    // Initialize beam with initial sequence
    double initial_score = evaluate(initial_sequence, objective);
    result.evaluations++;
    
    std::vector<Candidate> beam;
    beam.emplace_back(initial_sequence, initial_score);
    
    result.update_best(initial_sequence, initial_score);
    if (config_.track_history) {
        result.score_history.push_back(initial_score);
    }
    
    size_t iterations_without_improvement = 0;
    double prev_best_score = initial_score;
    
    // Main optimization loop
    while (!should_terminate(result)) {
        result.iterations++;
        
        // Expand beam (generate neighbors for all candidates)
        auto candidates = expand_beam(beam, objective, constraints);
        result.evaluations += candidates.size();
        
        // Select top-K candidates for next beam
        beam = select_beam(candidates);
        
        // Update best
        if (!beam.empty() && beam[0].score > result.best_score) {
            result.update_best(beam[0].sequence, beam[0].score);
        }
        
        // Track history
        if (config_.track_history) {
            result.score_history.push_back(result.best_score);
        }
        
        // Check for improvement
        if (result.best_score > prev_best_score + config_.min_improvement) {
            iterations_without_improvement = 0;
            prev_best_score = result.best_score;
        } else {
            iterations_without_improvement++;
        }
        
        // Check for convergence
        if (iterations_without_improvement >= config_.patience) {
            result.converged = true;
            result.termination_reason = "No improvement within patience window";
            break;
        }
        
        log_progress(result);
    }
    
    if (result.termination_reason.empty()) {
        result.termination_reason = "Reached iteration/evaluation limit";
    }
    
    return result;
}

std::vector<BeamSearch::Candidate> BeamSearch::expand_beam(
    const std::vector<Candidate>& beam,
    ObjectiveFunction& objective,
    const ConstraintEngine* constraints)
{
    std::vector<Candidate> candidates;
    
    // Generate neighbors for each candidate in beam
    for (const auto& candidate : beam) {
        for (size_t i = 0; i < algo_config_.expansion_factor; ++i) {
            auto result = mutation_engine_->apply_random_mutation(candidate.sequence);
            
            if (result.mutated_sequence.empty()) continue;
            if (!satisfies_constraints(result.mutated_sequence, constraints)) continue;
            
            // Check for duplicates if enabled
            if (algo_config_.prune_duplicates) {
                if (is_duplicate(result.mutated_sequence, candidates)) {
                    continue;
                }
            }
            
            // Check diversity if enabled
            if (algo_config_.diverse_beam) {
                if (!is_diverse(result.mutated_sequence, candidates)) {
                    continue;
                }
            }
            
            double score = evaluate(result.mutated_sequence, objective);
            candidates.emplace_back(result.mutated_sequence, score);
        }
    }
    
    // Protect against empty candidates
    if (candidates.empty() && !beam.empty()) {
        // Return original beam if no valid candidates generated
        return beam;
    }
    
    return candidates;
}

std::vector<BeamSearch::Candidate> BeamSearch::select_beam(std::vector<Candidate>& candidates) {
    // Sort by score (descending)
    std::sort(candidates.begin(), candidates.end(),
        [](const Candidate& a, const Candidate& b) {
            return a.score > b.score;
        });
    
    // Take top beam_width candidates
    size_t beam_size = std::min(algo_config_.beam_width, candidates.size());
    candidates.resize(beam_size);
    
    return candidates;
}

bool BeamSearch::is_diverse(const Sequence& seq, const std::vector<Candidate>& beam) {
    for (const auto& candidate : beam) {
        if (candidate.sequence.length() == seq.length()) {
            size_t diff_count = 0;
            for (size_t i = 0; i < seq.length(); ++i) {
                if (seq[i] != candidate.sequence[i]) {
                    ++diff_count;
                }
            }
            double similarity = 1.0 - (static_cast<double>(diff_count) / seq.length());
            if (similarity > (1.0 - algo_config_.diversity_threshold)) {
                return false;
            }
        }
    }
    return true;
}

bool BeamSearch::is_duplicate(const Sequence& seq, const std::vector<Candidate>& beam) {
    for (const auto& candidate : beam) {
        if (candidate.sequence.data() == seq.data()) {
            return true;
        }
    }
    return false;
}

} // namespace helixforge
