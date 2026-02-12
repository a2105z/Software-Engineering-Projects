#include "mutation_engine.hpp"
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <limits>

namespace helixforge {

// ============================================================================
// Mutation Implementation
// ============================================================================

std::string Mutation::to_string() const {
    std::ostringstream oss;
    
    switch (type) {
        case MutationType::SUBSTITUTION:
            oss << "SUB@" << position << ": " << original << " → " << mutated;
            break;
        case MutationType::INSERTION:
            oss << "INS@" << position << ": +" << mutated << " (length=" << length << ")";
            break;
        case MutationType::DELETION:
            oss << "DEL@" << position << ": -" << original << " (length=" << length << ")";
            break;
        case MutationType::INVERSION:
            oss << "INV@" << position << ": " << original << " ⇄ " << mutated << " (length=" << length << ")";
            break;
        case MutationType::DUPLICATION:
            oss << "DUP@" << position << ": " << original << " (length=" << length << ")";
            break;
        case MutationType::TRANSLOCATION:
            oss << "TRA@" << position << ": (length=" << length << ")";
            break;
    }
    
    return oss.str();
}

// ============================================================================
// MutationConfig Implementation
// ============================================================================

void MutationConfig::validate() const {
    double total = substitution_rate + insertion_rate + deletion_rate + 
                   inversion_rate + duplication_rate + translocation_rate;
    
    if (total > 1.0 + 1e-6) {
        throw std::invalid_argument("Mutation rates sum to more than 1.0");
    }
    
    if (maintain_length && (insertion_rate > 0.0 || deletion_rate > 0.0)) {
        throw std::invalid_argument("Cannot maintain length with insertions/deletions enabled");
    }
}

// ============================================================================
// MutationEngine Implementation
// ============================================================================

MutationEngine::MutationEngine(RNG& rng)
    : owned_rng_(nullptr), rng_(&rng) {}

MutationEngine::MutationEngine(uint64_t seed)
    : owned_rng_(std::make_unique<RNG>(seed)), rng_(owned_rng_.get()) {}

void MutationEngine::set_config(const MutationConfig& config) {
    config.validate();
    config_ = config;
}

MutationResult MutationEngine::apply_random_mutation(const Sequence& seq) {
    if (seq.empty()) {
        return MutationResult(seq, {}, false);
    }
    
    Mutation mut = generate_random_mutation(seq);
    Sequence mutated = apply_mutation(seq, mut);
    
    bool satisfied = check_constraints(mutated);
    
    return MutationResult(mutated, {mut}, satisfied);
}

MutationResult MutationEngine::apply_n_mutations(const Sequence& seq, size_t n) {
    Sequence current = seq;
    std::vector<Mutation> all_mutations;
    
    for (size_t i = 0; i < n; ++i) {
        if (current.empty()) break;
        
        Mutation mut = generate_random_mutation(current);
        current = apply_mutation(current, mut);
        all_mutations.push_back(mut);
    }
    
    bool satisfied = check_constraints(current);
    
    return MutationResult(current, all_mutations, satisfied);
}

MutationResult MutationEngine::apply_substitution(const Sequence& seq, Position pos, char new_base) {
    if (!is_valid_position(seq, pos)) {
        return MutationResult(seq, {}, false);
    }
    
    char old_base = seq[pos];
    Mutation mut(MutationType::SUBSTITUTION, pos, 
                 std::string(1, old_base), std::string(1, new_base), 1);
    
    Sequence mutated = apply_mutation(seq, mut);
    bool satisfied = check_constraints(mutated);
    
    return MutationResult(mutated, {mut}, satisfied);
}

MutationResult MutationEngine::apply_insertion(const Sequence& seq, Position pos, const std::string& insert_seq) {
    if (pos > seq.length()) {
        return MutationResult(seq, {}, false);
    }
    
    Mutation mut(MutationType::INSERTION, pos, "", insert_seq, insert_seq.length());
    Sequence mutated = apply_mutation(seq, mut);
    bool satisfied = check_constraints(mutated);
    
    return MutationResult(mutated, {mut}, satisfied);
}

MutationResult MutationEngine::apply_deletion(const Sequence& seq, Position pos, size_t length) {
    if (!is_valid_position(seq, pos) || pos + length > seq.length()) {
        return MutationResult(seq, {}, false);
    }
    
    std::string deleted = seq.data().substr(pos, length);
    Mutation mut(MutationType::DELETION, pos, deleted, "", length);
    
    Sequence mutated = apply_mutation(seq, mut);
    bool satisfied = check_constraints(mutated);
    
    return MutationResult(mutated, {mut}, satisfied);
}

MutationResult MutationEngine::apply_inversion(const Sequence& seq, Position pos, size_t length) {
    if (!is_valid_position(seq, pos) || pos + length > seq.length()) {
        return MutationResult(seq, {}, false);
    }
    
    std::string original = seq.data().substr(pos, length);
    Mutation mut(MutationType::INVERSION, pos, original, "", length);
    
    Sequence mutated = apply_mutation(seq, mut);
    bool satisfied = check_constraints(mutated);
    
    return MutationResult(mutated, {mut}, satisfied);
}

MutationResult MutationEngine::mutate_until_constraint(
    const Sequence& seq,
    const ConstraintEngine& constraints,
    size_t max_attempts)
{
    for (size_t attempt = 0; attempt < max_attempts; ++attempt) {
        auto result = apply_random_mutation(seq);
        
        if (constraints.satisfies_all(result.mutated_sequence)) {
            result.constraint_satisfied = true;
            return result;
        }
    }
    
    // Failed to find valid mutation
    MutationResult failed(seq);
    failed.constraint_satisfied = false;
    failed.failure_reason = "Could not find valid mutation within max attempts";
    return failed;
}

std::vector<MutationResult> MutationEngine::generate_mutant_population(
    const Sequence& seq,
    size_t population_size,
    size_t mutations_per_individual)
{
    std::vector<MutationResult> population;
    population.reserve(population_size);
    
    for (size_t i = 0; i < population_size; ++i) {
        auto result = apply_n_mutations(seq, mutations_per_individual);
        population.push_back(std::move(result));
    }
    
    return population;
}

std::vector<MutationResult> MutationEngine::mutational_scanning(
    const Sequence& seq,
    bool all_possible_substitutions)
{
    std::vector<MutationResult> results;
    
    const std::string bases = (seq.type() == SequenceType::DNA) ? "ACGT" : 
                              (seq.type() == SequenceType::RNA) ? "ACGU" : "ACDEFGHIKLMNPQRSTVWY";
    
    for (size_t pos = 0; pos < seq.length(); ++pos) {
        if (all_possible_substitutions) {
            // Try all possible bases at this position
            for (char base : bases) {
                if (base != seq[pos]) {
                    auto result = apply_substitution(seq, pos, base);
                    results.push_back(result);
                }
            }
        } else {
            // Just one random mutation at this position
            char new_base = get_random_base(seq.type(), seq[pos]);
            auto result = apply_substitution(seq, pos, new_base);
            results.push_back(result);
        }
    }
    
    return results;
}

void MutationEngine::set_constraint_checker(std::function<bool(const Sequence&)> checker) {
    constraint_checker_ = std::move(checker);
}

void MutationEngine::clear_constraint_checker() {
    constraint_checker_ = nullptr;
}

// ============================================================================
// Internal Methods
// ============================================================================

Mutation MutationEngine::generate_random_mutation(const Sequence& seq) {
    MutationType type = select_mutation_type();
    Position pos = get_random_position(seq);
    
    switch (type) {
        case MutationType::SUBSTITUTION:
            return generate_substitution(seq, pos);
        case MutationType::INSERTION:
            return generate_insertion(seq, pos);
        case MutationType::DELETION:
            return generate_deletion(seq, pos);
        case MutationType::INVERSION:
            return generate_inversion(seq, pos);
        default:
            return generate_substitution(seq, pos);
    }
}

Mutation MutationEngine::generate_substitution(const Sequence& seq, Position pos) {
    char old_base = seq[pos];
    char new_base;
    
    if (config_.codon_aware && config_.synonymous_only) {
        new_base = get_synonymous_codon_change(seq, pos - (pos % 3));
    } else {
        new_base = get_random_base(seq.type(), old_base);
    }
    
    return Mutation(MutationType::SUBSTITUTION, pos, 
                   std::string(1, old_base), std::string(1, new_base), 1);
}

Mutation MutationEngine::generate_insertion(const Sequence& seq, Position pos) {
    size_t length;
    
    if (config_.maintain_reading_frame) {
        length = 3; // Always insert codons
    } else {
        length = rng_->random_index(config_.max_insertion_length) + 1;
    }
    
    std::string insert_seq = get_random_sequence(seq.type(), length);
    
    return Mutation(MutationType::INSERTION, pos, "", insert_seq, length);
}

Mutation MutationEngine::generate_deletion(const Sequence& seq, Position pos) {
    size_t max_len = std::min(config_.max_deletion_length, seq.length() - pos);
    size_t length;
    
    if (config_.maintain_reading_frame) {
        length = 3 * (rng_->random_index(max_len / 3) + 1);
    } else {
        length = rng_->random_index(max_len) + 1;
    }
    
    std::string deleted = seq.data().substr(pos, length);
    
    return Mutation(MutationType::DELETION, pos, deleted, "", length);
}

Mutation MutationEngine::generate_inversion(const Sequence& seq, Position pos) {
    size_t max_len = std::min(config_.max_inversion_length, seq.length() - pos);
    size_t length = rng_->random_index(max_len) + 1;
    
    std::string original = seq.data().substr(pos, length);
    
    return Mutation(MutationType::INVERSION, pos, original, "", length);
}

Sequence MutationEngine::apply_mutation(const Sequence& seq, const Mutation& mut) {
    std::string new_data = seq.data();
    
    switch (mut.type) {
        case MutationType::SUBSTITUTION:
            new_data[mut.position] = mut.mutated[0];
            break;
            
        case MutationType::INSERTION:
            new_data.insert(mut.position, mut.mutated);
            break;
            
        case MutationType::DELETION:
            new_data.erase(mut.position, mut.length);
            break;
            
        case MutationType::INVERSION: {
            std::string segment = new_data.substr(mut.position, mut.length);
            std::reverse(segment.begin(), segment.end());
            new_data.replace(mut.position, mut.length, segment);
            break;
        }
            
        case MutationType::DUPLICATION:
            new_data.insert(mut.position + mut.length, 
                          new_data.substr(mut.position, mut.length));
            break;
            
        case MutationType::TRANSLOCATION:
            // Not implemented yet
            break;
    }
    
    return Sequence(new_data, seq.type(), seq.id() + "_mut", seq.description());
}

bool MutationEngine::is_valid_position(const Sequence& seq, Position pos) const {
    return pos < seq.length();
}

bool MutationEngine::check_constraints(const Sequence& seq) const {
    if (constraint_checker_) {
        return constraint_checker_(seq);
    }
    return true;
}

char MutationEngine::get_random_base(SequenceType type, char exclude) {
    std::string alphabet;
    
    switch (type) {
        case SequenceType::DNA:
            alphabet = "ACGT";
            break;
        case SequenceType::RNA:
            alphabet = "ACGU";
            break;
        case SequenceType::PROTEIN:
            alphabet = "ACDEFGHIKLMNPQRSTVWY";
            break;
        default:
            alphabet = "ACGT";
    }
    
    // Remove excluded character
    if (exclude != '\0') {
        alphabet.erase(std::remove(alphabet.begin(), alphabet.end(), exclude), alphabet.end());
    }
    
    if (alphabet.empty()) {
        throw std::runtime_error("No valid bases available for mutation");
    }
    
    return alphabet[rng_->random_index(alphabet.size())];
}

std::string MutationEngine::get_random_sequence(SequenceType type, size_t length) {
    std::string alphabet;
    
    switch (type) {
        case SequenceType::DNA:
            alphabet = "ACGT";
            break;
        case SequenceType::RNA:
            alphabet = "ACGU";
            break;
        case SequenceType::PROTEIN:
            alphabet = "ACDEFGHIKLMNPQRSTVWY";
            break;
        default:
            alphabet = "ACGT";
    }
    
    return rng_->random_string(alphabet, length);
}

Position MutationEngine::get_random_position(const Sequence& seq) {
    if (seq.empty()) {
        throw std::runtime_error("Cannot mutate empty sequence");
    }
    
    // Avoid start codon if configured
    if (config_.avoid_start_codon && seq.length() >= 3) {
        Position pos = rng_->random_index(seq.length() - 3) + 3;
        return pos;
    }
    
    return rng_->random_index(seq.length());
}

MutationType MutationEngine::select_mutation_type() {
    double r = rng_->random_double();
    double cumulative = 0.0;
    
    cumulative += config_.substitution_rate;
    if (r < cumulative) return MutationType::SUBSTITUTION;
    
    cumulative += config_.insertion_rate;
    if (r < cumulative) return MutationType::INSERTION;
    
    cumulative += config_.deletion_rate;
    if (r < cumulative) return MutationType::DELETION;
    
    cumulative += config_.inversion_rate;
    if (r < cumulative) return MutationType::INVERSION;
    
    cumulative += config_.duplication_rate;
    if (r < cumulative) return MutationType::DUPLICATION;
    
    cumulative += config_.translocation_rate;
    if (r < cumulative) return MutationType::TRANSLOCATION;
    
    // Default to substitution
    return MutationType::SUBSTITUTION;
}

char MutationEngine::get_synonymous_codon_change(const Sequence& seq, Position codon_start) {
    // Simplified: just return a random different base at the same position
    // A full implementation would look up the codon table and find synonymous codons
    return get_random_base(seq.type(), seq[codon_start]);
}

// ============================================================================
// Utility Functions
// ============================================================================

size_t hamming_distance(const Sequence& seq1, const Sequence& seq2) {
    if (seq1.length() != seq2.length()) {
        throw std::invalid_argument("Sequences must have equal length for Hamming distance");
    }
    
    size_t distance = 0;
    for (size_t i = 0; i < seq1.length(); ++i) {
        if (seq1[i] != seq2[i]) {
            ++distance;
        }
    }
    
    return distance;
}

size_t edit_distance(const Sequence& seq1, const Sequence& seq2) {
    const std::string& s1 = seq1.data();
    const std::string& s2 = seq2.data();
    
    size_t m = s1.length();
    size_t n = s2.length();
    
    // Create DP table
    std::vector<std::vector<size_t>> dp(m + 1, std::vector<size_t>(n + 1));
    
    // Initialize
    for (size_t i = 0; i <= m; ++i) dp[i][0] = i;
    for (size_t j = 0; j <= n; ++j) dp[0][j] = j;
    
    // Fill table
    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            if (s1[i - 1] == s2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = 1 + std::min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]});
            }
        }
    }
    
    return dp[m][n];
}

std::vector<Mutation> find_mutations(const Sequence& original, const Sequence& mutated) {
    std::vector<Mutation> mutations;
    
    // Simple implementation: just find substitutions
    if (original.length() == mutated.length()) {
        for (size_t i = 0; i < original.length(); ++i) {
            if (original[i] != mutated[i]) {
                mutations.emplace_back(
                    MutationType::SUBSTITUTION, i,
                    std::string(1, original[i]),
                    std::string(1, mutated[i]),
                    1
                );
            }
        }
    }
    
    return mutations;
}

std::vector<Sequence> generate_all_neighbors(const Sequence& seq) {
    std::vector<Sequence> neighbors;
    
    std::string bases = (seq.type() == SequenceType::DNA) ? "ACGT" : "ACGU";
    
    for (size_t pos = 0; pos < seq.length(); ++pos) {
        for (char base : bases) {
            if (base != seq[pos]) {
                std::string new_data = seq.data();
                new_data[pos] = base;
                neighbors.emplace_back(new_data, seq.type(), 
                                      seq.id() + "_neighbor", seq.description());
            }
        }
    }
    
    return neighbors;
}

double calculate_mutation_rate(const Sequence& original, const Sequence& mutated) {
    if (original.length() != mutated.length()) {
        return static_cast<double>(edit_distance(original, mutated)) / 
               std::max(original.length(), mutated.length());
    }
    
    return static_cast<double>(hamming_distance(original, mutated)) / original.length();
}

} // namespace helixforge
