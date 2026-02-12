#pragma once

#include <cstdint>
#include <random>
#include <vector>
#include <string>

namespace helixforge {

/**
 * Seeded random number generator for reproducible experiments
 * Wraps std::mt19937_64 with additional utility methods
 */
class RNG {
public:
    // Constructor with optional seed (uses random_device if seed not provided)
    explicit RNG(uint64_t seed = 0);
    
    // Reset with new seed
    void seed(uint64_t new_seed);
    
    // Get current seed
    uint64_t get_seed() const { return seed_; }
    
    // Generate random integer in range [min, max]
    int64_t random_int(int64_t min, int64_t max);
    
    // Generate random integer in range [0, max)
    size_t random_index(size_t max);
    
    // Generate random double in range [0.0, 1.0)
    double random_double();
    
    // Generate random double in range [min, max)
    double random_double(double min, double max);
    
    // Generate random boolean with given probability of true
    bool random_bool(double probability = 0.5);
    
    // Sample without replacement (returns indices)
    std::vector<size_t> sample_without_replacement(size_t population_size, size_t sample_size);
    
    // Sample with replacement (returns indices)
    std::vector<size_t> sample_with_replacement(size_t population_size, size_t sample_size);
    
    // Random choice from vector
    template<typename T>
    const T& choice(const std::vector<T>& items) {
        return items[random_index(items.size())];
    }
    
    template<typename T>
    T& choice(std::vector<T>& items) {
        return items[random_index(items.size())];
    }
    
    // Shuffle vector in place
    template<typename T>
    void shuffle(std::vector<T>& items) {
        std::shuffle(items.begin(), items.end(), engine_);
    }
    
    // Generate random string from alphabet
    std::string random_string(const std::string& alphabet, size_t length);
    
private:
    uint64_t seed_;
    std::mt19937_64 engine_;
    
    void initialize_from_seed(uint64_t s);
};

/**
 * Global RNG instance for convenience
 * Can be seeded for reproducibility
 */
class GlobalRNG {
public:
    static RNG& instance();
    static void set_seed(uint64_t seed);
    static uint64_t get_seed();
    
private:
    GlobalRNG() = default;
    static RNG rng_;
};

} // namespace helixforge
