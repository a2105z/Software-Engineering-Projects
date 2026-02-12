#pragma once

#include "optimizer.hpp"

namespace helixforge {

/**
 * Selection method for genetic algorithm
 */
enum class SelectionMethod {
    TOURNAMENT,
    ROULETTE,
    RANK,
    ELITIST
};

/**
 * Crossover method
 */
enum class CrossoverMethod {
    SINGLE_POINT,
    TWO_POINT,
    UNIFORM,
    NONE
};

/**
 * Genetic Algorithm Configuration
 */
struct GeneticAlgorithmConfig {
    size_t population_size = 100;
    double mutation_rate = 0.1;
    double crossover_rate = 0.7;
    size_t elitism_count = 2;             // Number of best individuals to preserve
    
    SelectionMethod selection = SelectionMethod::TOURNAMENT;
    size_t tournament_size = 3;
    
    CrossoverMethod crossover = CrossoverMethod::SINGLE_POINT;
    
    bool adaptive_mutation = false;       // Increase mutation if stuck
    double mutation_boost_factor = 2.0;
};

/**
 * Genetic Algorithm Optimizer
 * 
 * Evolves population through selection, crossover, and mutation.
 * Explores diverse regions of sequence space simultaneously.
 */
class GeneticAlgorithm : public Optimizer {
public:
    explicit GeneticAlgorithm(uint64_t seed = 0);
    
    OptimizationResult optimize(
        const Sequence& initial_sequence,
        ObjectiveFunction objective,
        const ConstraintEngine* constraints = nullptr
    ) override;
    
    std::string name() const override { return "GeneticAlgorithm"; }
    
    // Algorithm-specific configuration
    void set_algorithm_config(const GeneticAlgorithmConfig& config) {
        algo_config_ = config;
    }
    const GeneticAlgorithmConfig& get_algorithm_config() const {
        return algo_config_;
    }
    
private:
    GeneticAlgorithmConfig algo_config_;
    std::unique_ptr<RNG> rng_;
    
    // Individual in population
    struct Individual {
        Sequence sequence;
        double fitness;
        
        Individual(Sequence seq, double fit) 
            : sequence(std::move(seq)), fitness(fit) {}
        
        bool operator<(const Individual& other) const {
            return fitness > other.fitness;  // Sort descending by fitness
        }
    };
    
    // Initialize population
    std::vector<Individual> initialize_population(
        const Sequence& seed_sequence,
        ObjectiveFunction& objective,
        const ConstraintEngine* constraints
    );
    
    // Evaluate population
    void evaluate_population(
        std::vector<Individual>& population,
        ObjectiveFunction& objective
    );
    
    // Selection operators
    Individual tournament_selection(const std::vector<Individual>& population);
    Individual roulette_selection(const std::vector<Individual>& population);
    Individual rank_selection(const std::vector<Individual>& population);
    
    // Crossover operators
    std::pair<Sequence, Sequence> single_point_crossover(
        const Sequence& parent1, 
        const Sequence& parent2
    );
    std::pair<Sequence, Sequence> two_point_crossover(
        const Sequence& parent1,
        const Sequence& parent2
    );
    std::pair<Sequence, Sequence> uniform_crossover(
        const Sequence& parent1,
        const Sequence& parent2
    );
    
    // Mutation operator
    Sequence mutate(const Sequence& seq);
    
    // Create next generation
    std::vector<Individual> create_next_generation(
        const std::vector<Individual>& population,
        ObjectiveFunction& objective,
        const ConstraintEngine* constraints
    );
};

} // namespace helixforge
