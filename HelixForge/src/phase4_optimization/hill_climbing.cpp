#include "hill_climbing.hpp"
#include <algorithm>

namespace helixforge {

HillClimbing::HillClimbing(uint64_t seed)
    : rng_(std::make_unique<RNG>(seed)) {
    
    mutation_engine_ = std::make_shared<MutationEngine>(*rng_);
}

OptimizationResult HillClimbing::optimize(
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
    
    // Initialize
    Sequence current = initial_sequence;
    double current_score = evaluate(current, objective);
    result.evaluations++;
    
    result.update_best(current, current_score);
    if (config_.track_history) {
        result.score_history.push_back(current_score);
    }
    
    size_t iterations_without_improvement = 0;
    size_t restart_count = 0;
    
    // Main optimization loop
    while (!should_terminate(result)) {
        result.iterations++;
        
        // Generate neighbors
        auto neighbors = generate_neighbors(current, algo_config_.num_neighbors);
        
        // Find best neighbor
        double best_neighbor_score = current_score;
        Sequence best_neighbor = find_best_neighbor(
            neighbors, objective, constraints, best_neighbor_score
        );
        result.evaluations += neighbors.size();
        
        // Check for improvement
        if (best_neighbor_score > current_score + config_.min_improvement) {
            current = best_neighbor;
            current_score = best_neighbor_score;
            result.update_best(current, current_score);
            iterations_without_improvement = 0;
        } else {
            iterations_without_improvement++;
        }
        
        // Track history
        if (config_.track_history) {
            result.score_history.push_back(result.best_score);
        }
        
        // Check for convergence
        if (iterations_without_improvement >= config_.patience) {
            if (algo_config_.random_restart && restart_count < algo_config_.max_restarts) {
                current = random_restart_sequence(current);
                current_score = evaluate(current, objective);
                result.evaluations++;
                iterations_without_improvement = 0;
                restart_count++;
            } else {
                result.converged = true;
                result.termination_reason = "Converged to local optimum";
                break;
            }
        }
        
        log_progress(result);
    }
    
    if (result.termination_reason.empty()) {
        result.termination_reason = "Reached iteration/evaluation limit";
    }
    
    return result;
}

std::vector<Sequence> HillClimbing::generate_neighbors(const Sequence& seq, size_t count) {
    std::vector<Sequence> neighbors;
    neighbors.reserve(count);
    
    for (size_t i = 0; i < count; ++i) {
        auto result = mutation_engine_->apply_random_mutation(seq);
        if (!result.mutated_sequence.empty()) {
            neighbors.push_back(result.mutated_sequence);
        }
    }
    
    return neighbors;
}

Sequence HillClimbing::find_best_neighbor(
    const std::vector<Sequence>& neighbors,
    ObjectiveFunction& objective,
    const ConstraintEngine* constraints,
    double& best_score)
{
    Sequence best = neighbors.empty() ? Sequence() : neighbors[0];
    best_score = -std::numeric_limits<double>::infinity();
    
    for (const auto& neighbor : neighbors) {
        if (!satisfies_constraints(neighbor, constraints)) {
            continue;
        }
        
        double score = evaluate(neighbor, objective);
        if (score > best_score) {
            best_score = score;
            best = neighbor;
        }
    }
    
    return best;
}

Sequence HillClimbing::random_restart_sequence(const Sequence& current) {
    // Apply multiple mutations to jump to different region
    auto result = mutation_engine_->apply_n_mutations(current, 5);
    return result.mutated_sequence;
}

} // namespace helixforge
