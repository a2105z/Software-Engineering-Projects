#pragma once

#include "optimizer.hpp"

namespace helixforge {

/**
 * Cooling schedule types
 */
enum class CoolingSchedule {
    LINEAR,
    EXPONENTIAL,
    LOGARITHMIC,
    ADAPTIVE
};

/**
 * Simulated Annealing Configuration
 */
struct SimulatedAnnealingConfig {
    double initial_temperature = 100.0;
    double final_temperature = 0.01;
    CoolingSchedule cooling_schedule = CoolingSchedule::EXPONENTIAL;
    double cooling_rate = 0.95;           // For exponential cooling
    size_t steps_per_temperature = 10;    // Iterations at each temperature
    
    // Adaptive parameters
    double target_acceptance_rate = 0.3;  // For adaptive cooling
    size_t adaptation_window = 100;
};

/**
 * Simulated Annealing Optimizer
 * 
 * Probabilistically accepts worse solutions to escape local optima.
 * Temperature gradually decreases, reducing exploration over time.
 */
class SimulatedAnnealing : public Optimizer {
public:
    explicit SimulatedAnnealing(uint64_t seed = 0);
    
    OptimizationResult optimize(
        const Sequence& initial_sequence,
        ObjectiveFunction objective,
        const ConstraintEngine* constraints = nullptr
    ) override;
    
    std::string name() const override { return "SimulatedAnnealing"; }
    
    // Algorithm-specific configuration
    void set_algorithm_config(const SimulatedAnnealingConfig& config) {
        algo_config_ = config;
    }
    const SimulatedAnnealingConfig& get_algorithm_config() const {
        return algo_config_;
    }
    
private:
    SimulatedAnnealingConfig algo_config_;
    std::unique_ptr<RNG> rng_;
    
    // Temperature schedule
    double calculate_temperature(size_t iteration) const;
    
    // Acceptance probability (Metropolis criterion)
    bool accept_transition(double current_score, double neighbor_score, double temperature);
    
    // Generate single neighbor
    Sequence generate_neighbor(const Sequence& seq);
};

} // namespace helixforge
