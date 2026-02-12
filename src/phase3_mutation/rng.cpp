#include "rng.hpp"
#include <algorithm>
#include <stdexcept>

namespace helixforge {

// ============================================================================
// RNG Implementation
// ============================================================================

RNG::RNG(uint64_t seed) {
    initialize_from_seed(seed);
}

void RNG::initialize_from_seed(uint64_t s) {
    if (s == 0) {
        // Use random_device for true randomness
        std::random_device rd;
        seed_ = rd();
    } else {
        seed_ = s;
    }
    engine_.seed(seed_);
}

void RNG::seed(uint64_t new_seed) {
    initialize_from_seed(new_seed);
}

int64_t RNG::random_int(int64_t min, int64_t max) {
    if (min > max) {
        throw std::invalid_argument("RNG::random_int: min must be <= max");
    }
    std::uniform_int_distribution<int64_t> dist(min, max);
    return dist(engine_);
}

size_t RNG::random_index(size_t max) {
    if (max == 0) {
        throw std::invalid_argument("RNG::random_index: max must be > 0");
    }
    std::uniform_int_distribution<size_t> dist(0, max - 1);
    return dist(engine_);
}

double RNG::random_double() {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(engine_);
}

double RNG::random_double(double min, double max) {
    if (min > max) {
        throw std::invalid_argument("RNG::random_double: min must be <= max");
    }
    std::uniform_real_distribution<double> dist(min, max);
    return dist(engine_);
}

bool RNG::random_bool(double probability) {
    if (probability < 0.0 || probability > 1.0) {
        throw std::invalid_argument("RNG::random_bool: probability must be in [0, 1]");
    }
    return random_double() < probability;
}

std::vector<size_t> RNG::sample_without_replacement(size_t population_size, size_t sample_size) {
    if (sample_size > population_size) {
        throw std::invalid_argument("Sample size cannot exceed population size");
    }
    
    // Create population indices
    std::vector<size_t> population(population_size);
    for (size_t i = 0; i < population_size; ++i) {
        population[i] = i;
    }
    
    // Shuffle and take first sample_size elements
    shuffle(population);
    population.resize(sample_size);
    
    return population;
}

std::vector<size_t> RNG::sample_with_replacement(size_t population_size, size_t sample_size) {
    std::vector<size_t> sample(sample_size);
    for (size_t i = 0; i < sample_size; ++i) {
        sample[i] = random_index(population_size);
    }
    return sample;
}

std::string RNG::random_string(const std::string& alphabet, size_t length) {
    if (alphabet.empty()) {
        throw std::invalid_argument("Alphabet cannot be empty");
    }
    
    std::string result;
    result.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        result += alphabet[random_index(alphabet.size())];
    }
    
    return result;
}

// ============================================================================
// GlobalRNG Implementation
// ============================================================================

RNG GlobalRNG::rng_(0);

RNG& GlobalRNG::instance() {
    return rng_;
}

void GlobalRNG::set_seed(uint64_t seed) {
    rng_.seed(seed);
}

uint64_t GlobalRNG::get_seed() {
    return rng_.get_seed();
}

} // namespace helixforge
