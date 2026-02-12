#include "genetic_algorithm.hpp"
#include <algorithm>
#include <numeric>
#include <limits>

namespace helixforge {

GeneticAlgorithm::GeneticAlgorithm(uint64_t seed)
    : rng_(std::make_unique<RNG>(seed)) {
    
    mutation_engine_ = std::make_shared<MutationEngine>(*rng_);
}

OptimizationResult GeneticAlgorithm::optimize(
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
    
    // Initialize population
    auto population = initialize_population(initial_sequence, objective, constraints);
    result.evaluations += population.size();
    
    // Track best
    if (!population.empty()) {
        result.update_best(population[0].sequence, population[0].fitness);
        if (config_.track_history) {
            result.score_history.push_back(population[0].fitness);
        }
    }
    
    size_t iterations_without_improvement = 0;
    double prev_best_score = result.best_score;
    
    // Main evolutionary loop
    while (!should_terminate(result)) {
        result.iterations++;
        
        // Create next generation
        population = create_next_generation(population, objective, constraints);
        result.evaluations += population.size();
        
        // Update best
        if (!population.empty() && population[0].fitness > result.best_score) {
            result.update_best(population[0].sequence, population[0].fitness);
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

std::vector<GeneticAlgorithm::Individual> GeneticAlgorithm::initialize_population(
    const Sequence& seed_sequence,
    ObjectiveFunction& objective,
    const ConstraintEngine* constraints)
{
    std::vector<Individual> population;
    
    // Add seed sequence
    double seed_fitness = evaluate(seed_sequence, objective);
    population.emplace_back(seed_sequence, seed_fitness);
    
    // Generate mutants
    size_t attempts = 0;
    size_t max_attempts = algo_config_.population_size * 10;
    
    while (population.size() < algo_config_.population_size && attempts < max_attempts) {
        attempts++;
        
        // Generate variant
        size_t num_mutations = rng_->random_index(5) + 1;
        auto result = mutation_engine_->apply_n_mutations(seed_sequence, num_mutations);
        
        if (!satisfies_constraints(result.mutated_sequence, constraints)) {
            continue;
        }
        
        double fitness = evaluate(result.mutated_sequence, objective);
        population.emplace_back(result.mutated_sequence, fitness);
    }
    
    // Sort by fitness
    std::sort(population.begin(), population.end());
    
    return population;
}

void GeneticAlgorithm::evaluate_population(
    std::vector<Individual>& population,
    ObjectiveFunction& objective)
{
    for (auto& individual : population) {
        individual.fitness = evaluate(individual.sequence, objective);
    }
    
    // Sort by fitness
    std::sort(population.begin(), population.end());
}

GeneticAlgorithm::Individual GeneticAlgorithm::tournament_selection(
    const std::vector<Individual>& population)
{
    // Select random individuals for tournament
    std::vector<const Individual*> tournament;
    for (size_t i = 0; i < algo_config_.tournament_size; ++i) {
        size_t idx = rng_->random_index(population.size());
        tournament.push_back(&population[idx]);
    }
    
    // Return best from tournament
    const Individual* best = tournament[0];
    for (const auto* ind : tournament) {
        if (ind->fitness > best->fitness) {
            best = ind;
        }
    }
    
    return *best;
}

GeneticAlgorithm::Individual GeneticAlgorithm::roulette_selection(
    const std::vector<Individual>& population)
{
    // Calculate total fitness
    double total_fitness = 0.0;
    for (const auto& ind : population) {
        total_fitness += std::max(0.0, ind.fitness);
    }
    
    if (total_fitness <= 0.0) {
        return population[rng_->random_index(population.size())];
    }
    
    // Spin roulette
    double spin = rng_->random_double() * total_fitness;
    double cumulative = 0.0;
    
    for (const auto& ind : population) {
        cumulative += std::max(0.0, ind.fitness);
        if (cumulative >= spin) {
            return ind;
        }
    }
    
    return population.back();
}

GeneticAlgorithm::Individual GeneticAlgorithm::rank_selection(
    const std::vector<Individual>& population)
{
    // Use rank instead of raw fitness
    size_t total_rank = (population.size() * (population.size() + 1)) / 2;
    size_t spin = rng_->random_index(total_rank);
    
    size_t cumulative = 0;
    for (size_t i = 0; i < population.size(); ++i) {
        cumulative += (population.size() - i);
        if (cumulative >= spin) {
            return population[i];
        }
    }
    
    return population.back();
}

std::pair<Sequence, Sequence> GeneticAlgorithm::single_point_crossover(
    const Sequence& parent1,
    const Sequence& parent2)
{
    size_t min_len = std::min(parent1.length(), parent2.length());
    if (min_len < 2) {
        return {parent1, parent2};
    }
    
    size_t crossover_point = rng_->random_index(min_len - 1) + 1;
    
    std::string child1_data = parent1.data().substr(0, crossover_point) +
                              parent2.data().substr(crossover_point);
    std::string child2_data = parent2.data().substr(0, crossover_point) +
                              parent1.data().substr(crossover_point);
    
    return {
        Sequence(child1_data, parent1.type()),
        Sequence(child2_data, parent2.type())
    };
}

std::pair<Sequence, Sequence> GeneticAlgorithm::two_point_crossover(
    const Sequence& parent1,
    const Sequence& parent2)
{
    size_t min_len = std::min(parent1.length(), parent2.length());
    if (min_len < 3) {
        return single_point_crossover(parent1, parent2);
    }
    
    size_t point1 = rng_->random_index(min_len - 2) + 1;
    size_t point2 = rng_->random_index(min_len - point1 - 1) + point1 + 1;
    
    std::string child1_data = parent1.data().substr(0, point1) +
                              parent2.data().substr(point1, point2 - point1) +
                              parent1.data().substr(point2);
    
    std::string child2_data = parent2.data().substr(0, point1) +
                              parent1.data().substr(point1, point2 - point1) +
                              parent2.data().substr(point2);
    
    return {
        Sequence(child1_data, parent1.type()),
        Sequence(child2_data, parent2.type())
    };
}

std::pair<Sequence, Sequence> GeneticAlgorithm::uniform_crossover(
    const Sequence& parent1,
    const Sequence& parent2)
{
    size_t min_len = std::min(parent1.length(), parent2.length());
    
    std::string child1_data, child2_data;
    child1_data.reserve(min_len);
    child2_data.reserve(min_len);
    
    for (size_t i = 0; i < min_len; ++i) {
        if (rng_->random_bool()) {
            child1_data += parent1[i];
            child2_data += parent2[i];
        } else {
            child1_data += parent2[i];
            child2_data += parent1[i];
        }
    }
    
    return {
        Sequence(child1_data, parent1.type()),
        Sequence(child2_data, parent2.type())
    };
}

Sequence GeneticAlgorithm::mutate(const Sequence& seq) {
    if (rng_->random_double() < algo_config_.mutation_rate) {
        auto result = mutation_engine_->apply_random_mutation(seq);
        return result.mutated_sequence;
    }
    return seq;
}

std::vector<GeneticAlgorithm::Individual> GeneticAlgorithm::create_next_generation(
    const std::vector<Individual>& population,
    ObjectiveFunction& objective,
    const ConstraintEngine* constraints)
{
    std::vector<Individual> next_generation;
    
    // Elitism: preserve best individuals
    for (size_t i = 0; i < algo_config_.elitism_count && i < population.size(); ++i) {
        next_generation.push_back(population[i]);
    }
    
    // Generate offspring
    while (next_generation.size() < algo_config_.population_size) {
        // Selection
        Individual parent1 = tournament_selection(population);
        Individual parent2 = tournament_selection(population);
        
        // Crossover
        Sequence child1, child2;
        if (rng_->random_double() < algo_config_.crossover_rate) {
            auto children = single_point_crossover(parent1.sequence, parent2.sequence);
            child1 = children.first;
            child2 = children.second;
        } else {
            child1 = parent1.sequence;
            child2 = parent2.sequence;
        }
        
        // Mutation
        child1 = mutate(child1);
        child2 = mutate(child2);
        
        // Add to next generation if valid
        if (satisfies_constraints(child1, constraints)) {
            double fitness = evaluate(child1, objective);
            next_generation.emplace_back(child1, fitness);
        }
        
        if (next_generation.size() < algo_config_.population_size &&
            satisfies_constraints(child2, constraints)) {
            double fitness = evaluate(child2, objective);
            next_generation.emplace_back(child2, fitness);
        }
    }
    
    // Sort by fitness
    std::sort(next_generation.begin(), next_generation.end());
    
    return next_generation;
}

} // namespace helixforge
