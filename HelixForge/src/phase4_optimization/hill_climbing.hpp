#pragma once

#include "optimizer.hpp"

namespace helixforge {

/**
 * Hill Climbing Configuration
 */
struct HillClimbingConfig {
    bool stochastic = false;       // If true, accept any improvement; if false, take best neighbor
    size_t num_neighbors = 10;     // Number of neighbors to generate per iteration
    bool random_restart = false;   // Enable random restarts on convergence
    size_t max_restarts = 3;       // Maximum number of restarts
};

/**
 * Hill Climbing Optimizer
 * 
 * Iteratively moves to better neighbors until local optimum is reached.
 * Simple but effective for smooth fitness landscapes.
 */
class HillClimbing : public Optimizer {
public:
    explicit HillClimbing(uint64_t seed = 0);
    
    OptimizationResult optimize(
        const Sequence& initial_sequence,
        ObjectiveFunction objective,
        const ConstraintEngine* constraints = nullptr
    ) override;
    
    std::string name() const override { return "HillClimbing"; }
    
    // Algorithm-specific configuration
    void set_algorithm_config(const HillClimbingConfig& config) { 
        algo_config_ = config; 
    }
    const HillClimbingConfig& get_algorithm_config() const { 
        return algo_config_; 
    }
    
private:
    HillClimbingConfig algo_config_;
    std::unique_ptr<RNG> rng_;
    
    // Generate neighbors for current sequence
    std::vector<Sequence> generate_neighbors(const Sequence& seq, size_t count);
    
    // Find best neighbor
    Sequence find_best_neighbor(
        const std::vector<Sequence>& neighbors,
        ObjectiveFunction& objective,
        const ConstraintEngine* constraints,
        double& best_score
    );
    
    // Random restart
    Sequence random_restart_sequence(const Sequence& current);
};

} // namespace helixforge
