#pragma once

#include "../phase1_core/sequence.hpp"
#include "../phase2_evaluation/constraint_engine.hpp"
#include "rng.hpp"
#include <vector>
#include <memory>
#include <optional>
#include <functional>

namespace helixforge {

// ============================================================================
// Mutation Types
// ============================================================================

enum class MutationType {
    SUBSTITUTION,    // Point mutation (single base change)
    INSERTION,       // Insert one or more bases
    DELETION,        // Delete one or more bases
    INVERSION,       // Reverse a segment
    DUPLICATION,     // Duplicate a segment
    TRANSLOCATION    // Move a segment to another position
};

/**
 * Description of a single mutation
 */
struct Mutation {
    MutationType type;
    Position position;         // Where the mutation occurs
    std::string original;      // Original sequence content
    std::string mutated;       // Mutated sequence content
    size_t length;             // Length of affected region
    
    Mutation(MutationType t, Position pos, std::string orig, std::string mut, size_t len = 1)
        : type(t), position(pos), original(std::move(orig)), mutated(std::move(mut)), length(len) {}
    
    std::string to_string() const;
};

/**
 * Result of applying mutations
 */
struct MutationResult {
    Sequence mutated_sequence;
    std::vector<Mutation> mutations;
    bool constraint_satisfied;
    std::string failure_reason;
    
    MutationResult(Sequence seq, std::vector<Mutation> muts = {}, bool satisfied = true)
        : mutated_sequence(std::move(seq))
        , mutations(std::move(muts))
        , constraint_satisfied(satisfied) {}
};

// ============================================================================
// Mutation Engine Configuration
// ============================================================================

struct MutationConfig {
    // Probabilities for each mutation type (must sum to <= 1.0)
    double substitution_rate = 0.7;
    double insertion_rate = 0.15;
    double deletion_rate = 0.15;
    double inversion_rate = 0.0;
    double duplication_rate = 0.0;
    double translocation_rate = 0.0;
    
    // Size parameters
    size_t max_insertion_length = 5;
    size_t max_deletion_length = 5;
    size_t max_inversion_length = 10;
    
    // Constraints
    bool maintain_length = false;        // Keep sequence length constant
    bool maintain_reading_frame = false; // Only mutations divisible by 3
    bool avoid_start_codon = true;       // Don't mutate start codon
    bool avoid_stop_codons = true;       // Don't create premature stops
    
    // Codon-aware mutations (for coding sequences)
    bool codon_aware = false;            // Mutate at codon level
    bool synonymous_only = false;        // Only synonymous substitutions
    
    void validate() const;
};

// ============================================================================
// Mutation Engine
// ============================================================================

/**
 * Engine for applying controlled mutations to sequences
 */
class MutationEngine {
public:
    explicit MutationEngine(RNG& rng);
    explicit MutationEngine(uint64_t seed = 0);
    
    // Configure mutation parameters
    void set_config(const MutationConfig& config);
    const MutationConfig& get_config() const { return config_; }
    
    // Apply a single random mutation
    MutationResult apply_random_mutation(const Sequence& seq);
    
    // Apply N random mutations
    MutationResult apply_n_mutations(const Sequence& seq, size_t n);
    
    // Apply specific mutation type
    MutationResult apply_substitution(const Sequence& seq, Position pos, char new_base);
    MutationResult apply_insertion(const Sequence& seq, Position pos, const std::string& insert_seq);
    MutationResult apply_deletion(const Sequence& seq, Position pos, size_t length);
    MutationResult apply_inversion(const Sequence& seq, Position pos, size_t length);
    
    // Mutate until constraint is satisfied (with max attempts)
    MutationResult mutate_until_constraint(
        const Sequence& seq,
        const ConstraintEngine& constraints,
        size_t max_attempts = 1000
    );
    
    // Generate a population of mutants
    std::vector<MutationResult> generate_mutant_population(
        const Sequence& seq,
        size_t population_size,
        size_t mutations_per_individual
    );
    
    // Mutational scanning (mutate every position)
    std::vector<MutationResult> mutational_scanning(
        const Sequence& seq,
        bool all_possible_substitutions = false
    );
    
    // Add custom constraint checker
    void set_constraint_checker(std::function<bool(const Sequence&)> checker);
    void clear_constraint_checker();
    
    // RNG access
    RNG& rng() { return *rng_; }
    
private:
    std::unique_ptr<RNG> owned_rng_;
    RNG* rng_;
    MutationConfig config_;
    std::function<bool(const Sequence&)> constraint_checker_;
    
    // Internal mutation methods
    Mutation generate_random_mutation(const Sequence& seq);
    Mutation generate_substitution(const Sequence& seq, Position pos);
    Mutation generate_insertion(const Sequence& seq, Position pos);
    Mutation generate_deletion(const Sequence& seq, Position pos);
    Mutation generate_inversion(const Sequence& seq, Position pos);
    
    // Apply mutation to sequence
    Sequence apply_mutation(const Sequence& seq, const Mutation& mut);
    
    // Validation
    bool is_valid_position(const Sequence& seq, Position pos) const;
    bool check_constraints(const Sequence& seq) const;
    
    // Helper methods
    char get_random_base(SequenceType type, char exclude = '\0');
    std::string get_random_sequence(SequenceType type, size_t length);
    Position get_random_position(const Sequence& seq);
    MutationType select_mutation_type();
    
    // Codon-aware helpers
    char get_synonymous_codon_change(const Sequence& seq, Position codon_start);
};

// ============================================================================
// Mutation Utilities
// ============================================================================

/**
 * Calculate Hamming distance between two sequences
 */
size_t hamming_distance(const Sequence& seq1, const Sequence& seq2);

/**
 * Calculate edit distance (Levenshtein) between two sequences
 */
size_t edit_distance(const Sequence& seq1, const Sequence& seq2);

/**
 * Identify all differences between two sequences
 */
std::vector<Mutation> find_mutations(const Sequence& original, const Sequence& mutated);

/**
 * Generate all possible single substitutions of a sequence
 */
std::vector<Sequence> generate_all_neighbors(const Sequence& seq);

/**
 * Calculate mutation rate (mutations per base)
 */
double calculate_mutation_rate(const Sequence& original, const Sequence& mutated);

} // namespace helixforge
