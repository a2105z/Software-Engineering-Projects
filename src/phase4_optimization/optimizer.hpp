#pragma once

#include "../phase1_core/sequence.hpp"
#include "../phase2_evaluation/scoring.hpp"
#include "../phase2_evaluation/constraint_engine.hpp"
#include "../phase3_mutation/mutation_engine.hpp"
#include <functional>
#include <memory>
#include <vector>
#include <string>
#include <optional>

namespace helixforge {

// ============================================================================
// Objective Function Types
// ============================================================================

/**
 * Objective function: maps sequence to fitness score
 * Higher score = better fitness
 */
using ObjectiveFunction = std::function<double(const Sequence&)>;

/**
 * Predefined objective types
 */
enum class ObjectiveType {
    MAXIMIZE_GC,           // Maximize GC content
    MINIMIZE_GC,           // Minimize GC content
    TARGET_GC,             // Hit specific GC target
    MAXIMIZE_CAI,          // Maximize codon adaptation index
    MINIMIZE_RARE_CODONS,  // Minimize rare codon usage
    MAXIMIZE_EXPRESSION,   // Optimize for expression (CAI + GC)
    MINIMIZE_STRUCTURE,    // Minimize secondary structure
    CUSTOM                 // User-defined objective
};

// ============================================================================
// Optimization Result
// ============================================================================

/**
 * Tracks optimization progress and results
 */
struct OptimizationResult {
    Sequence best_sequence;
    double best_score;
    size_t iterations;
    size_t evaluations;
    bool converged;
    std::string termination_reason;
    
    // Trajectory tracking
    std::vector<double> score_history;
    std::vector<size_t> evaluation_history;
    
    OptimizationResult()
        : best_score(-std::numeric_limits<double>::infinity())
        , iterations(0)
        , evaluations(0)
        , converged(false) {}
    
    void update_best(const Sequence& seq, double score);
    std::string to_string() const;
    std::string to_json() const;
};

// ============================================================================
// Optimization Configuration
// ============================================================================

struct OptimizationConfig {
    // Termination criteria
    size_t max_iterations = 1000;
    size_t max_evaluations = 10000;
    double target_score = std::numeric_limits<double>::infinity();
    size_t patience = 100;  // Iterations without improvement before stopping
    double min_improvement = 1e-6;  // Minimum improvement to count as progress
    
    // Constraint handling
    bool enforce_constraints = true;
    size_t max_constraint_attempts = 100;
    
    // Logging
    bool verbose = false;
    size_t log_frequency = 100;
    
    // Tracking
    bool track_history = true;
    
    void validate() const;
};

// ============================================================================
// Base Optimizer
// ============================================================================

/**
 * Abstract base class for all optimization algorithms
 */
class Optimizer {
public:
    virtual ~Optimizer() = default;
    
    // Main optimization method
    virtual OptimizationResult optimize(
        const Sequence& initial_sequence,
        ObjectiveFunction objective,
        const ConstraintEngine* constraints = nullptr
    ) = 0;
    
    // Algorithm name
    virtual std::string name() const = 0;
    
    // Configuration
    void set_config(const OptimizationConfig& config) { config_ = config; }
    const OptimizationConfig& get_config() const { return config_; }
    
    // Set mutation engine
    void set_mutation_engine(std::shared_ptr<MutationEngine> engine) { 
        mutation_engine_ = engine; 
    }
    
protected:
    OptimizationConfig config_;
    std::shared_ptr<MutationEngine> mutation_engine_;
    
    // Helper methods
    double evaluate(const Sequence& seq, ObjectiveFunction& objective);
    bool satisfies_constraints(const Sequence& seq, const ConstraintEngine* constraints);
    bool should_terminate(const OptimizationResult& result) const;
    void log_progress(const OptimizationResult& result) const;
};

// ============================================================================
// Objective Function Factory
// ============================================================================

/**
 * Creates predefined objective functions
 */
class ObjectiveFactory {
public:
    static ObjectiveFunction create(ObjectiveType type, double parameter = 0.0);
    
    // Specific objective builders
    static ObjectiveFunction maximize_gc();
    static ObjectiveFunction minimize_gc();
    static ObjectiveFunction target_gc(double target);
    static ObjectiveFunction maximize_cai();
    static ObjectiveFunction minimize_rare_codons();
    static ObjectiveFunction maximize_expression();
    static ObjectiveFunction minimize_structure();
    
    // Composite objectives (weighted sum)
    static ObjectiveFunction weighted_sum(
        const std::vector<ObjectiveFunction>& objectives,
        const std::vector<double>& weights
    );
};

// ============================================================================
// Optimizer Factory
// ============================================================================

enum class OptimizerType {
    HILL_CLIMBING,
    SIMULATED_ANNEALING,
    BEAM_SEARCH,
    GENETIC_ALGORITHM
};

/**
 * Factory for creating optimizers
 */
class OptimizerFactory {
public:
    static std::unique_ptr<Optimizer> create(OptimizerType type, uint64_t seed = 0);
};

} // namespace helixforge
