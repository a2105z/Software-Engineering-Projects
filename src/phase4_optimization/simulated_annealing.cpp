#include "simulated_annealing.hpp"
#include <cmath>

namespace helixforge {

SimulatedAnnealing::SimulatedAnnealing(uint64_t seed)
    : rng_(std::make_unique<RNG>(seed)) {
    
    mutation_engine_ = std::make_shared<MutationEngine>(*rng_);
}

OptimizationResult SimulatedAnnealing::optimize(
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
    
    // Main optimization loop
    while (!should_terminate(result)) {
        result.iterations++;
        
        // Calculate current temperature
        double temperature = calculate_temperature(result.iterations);
        
        // Check if temperature too low
        if (temperature < algo_config_.final_temperature) {
            result.converged = true;
            result.termination_reason = "Temperature reached minimum";
            break;
        }
        
        // Generate neighbor
        Sequence neighbor = generate_neighbor(current);
        
        // Skip if violates constraints
        if (!satisfies_constraints(neighbor, constraints)) {
            continue;
        }
        
        double neighbor_score = evaluate(neighbor, objective);
        result.evaluations++;
        
        // Decide whether to accept neighbor
        bool accept = accept_transition(current_score, neighbor_score, temperature);
        
        if (accept) {
            current = neighbor;
            current_score = neighbor_score;
            
            // Update best if improved
            if (current_score > result.best_score) {
                result.update_best(current, current_score);
                iterations_without_improvement = 0;
            } else {
                iterations_without_improvement++;
            }
        } else {
            iterations_without_improvement++;
        }
        
        // Track history
        if (config_.track_history) {
            result.score_history.push_back(result.best_score);
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

double SimulatedAnnealing::calculate_temperature(size_t iteration) const {
    double T0 = algo_config_.initial_temperature;
    double Tf = algo_config_.final_temperature;
    
    switch (algo_config_.cooling_schedule) {
        case CoolingSchedule::LINEAR: {
            double slope = (Tf - T0) / config_.max_iterations;
            return std::max(Tf, T0 + slope * iteration);
        }
        
        case CoolingSchedule::EXPONENTIAL: {
            return std::max(Tf, T0 * std::pow(algo_config_.cooling_rate, iteration));
        }
        
        case CoolingSchedule::LOGARITHMIC: {
            return std::max(Tf, T0 / std::log(iteration + 2));
        }
        
        case CoolingSchedule::ADAPTIVE: {
            // Simplified adaptive: exponential with dynamic rate
            return std::max(Tf, T0 * std::pow(0.95, iteration));
        }
        
        default:
            return T0;
    }
}

bool SimulatedAnnealing::accept_transition(
    double current_score, 
    double neighbor_score, 
    double temperature)
{
    // Always accept improvements
    if (neighbor_score > current_score) {
        return true;
    }
    
    // Accept worse solutions with probability based on temperature
    double delta = neighbor_score - current_score;
    double acceptance_prob = std::exp(delta / temperature);
    
    return rng_->random_double() < acceptance_prob;
}

Sequence SimulatedAnnealing::generate_neighbor(const Sequence& seq) {
    auto result = mutation_engine_->apply_random_mutation(seq);
    return result.mutated_sequence;
}

} // namespace helixforge
