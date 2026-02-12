#pragma once

#include "optimizer.hpp"
#include <queue>

namespace helixforge {

/**
 * Beam Search Configuration
 */
struct BeamSearchConfig {
    size_t beam_width = 10;              // Number of candidates to maintain
    size_t expansion_factor = 5;         // Neighbors per candidate
    bool diverse_beam = false;           // Enforce diversity in beam
    double diversity_threshold = 0.1;    // Minimum distance between beam members
    bool prune_duplicates = true;        // Remove duplicate sequences
};

/**
 * Beam Search Optimizer
 * 
 * Maintains a "beam" of best candidates, expands all of them,
 * and keeps top-K for next iteration. Explores multiple promising paths.
 */
class BeamSearch : public Optimizer {
public:
    explicit BeamSearch(uint64_t seed = 0);
    
    OptimizationResult optimize(
        const Sequence& initial_sequence,
        ObjectiveFunction objective,
        const ConstraintEngine* constraints = nullptr
    ) override;
    
    std::string name() const override { return "BeamSearch"; }
    
    // Algorithm-specific configuration
    void set_algorithm_config(const BeamSearchConfig& config) {
        algo_config_ = config;
    }
    const BeamSearchConfig& get_algorithm_config() const {
        return algo_config_;
    }
    
private:
    BeamSearchConfig algo_config_;
    std::unique_ptr<RNG> rng_;
    
    // Candidate with score
    struct Candidate {
        Sequence sequence;
        double score;
        
        Candidate(Sequence seq, double s) : sequence(std::move(seq)), score(s) {}
        
        bool operator<(const Candidate& other) const {
            return score < other.score;  // Min-heap, we want max
        }
    };
    
    // Expand beam (generate neighbors for all candidates)
    std::vector<Candidate> expand_beam(
        const std::vector<Candidate>& beam,
        ObjectiveFunction& objective,
        const ConstraintEngine* constraints
    );
    
    // Select top-K candidates
    std::vector<Candidate> select_beam(std::vector<Candidate>& candidates);
    
    // Check if candidate is diverse enough
    bool is_diverse(const Sequence& seq, const std::vector<Candidate>& beam);
    
    // Check if sequence already in beam
    bool is_duplicate(const Sequence& seq, const std::vector<Candidate>& beam);
};

} // namespace helixforge
