#pragma once

#include "types.hpp"
#include <string>
#include <string_view>
#include <unordered_set>
#include <unordered_map>
#include <cctype>
#include <algorithm>

namespace helixforge {

// DNA alphabet (standard nucleotides)
constexpr char DNA_BASES[] = {'A', 'C', 'G', 'T'};
constexpr char DNA_BASES_EXTENDED[] = {'A', 'C', 'G', 'T', 'N', 'R', 'Y', 'S', 'W', 'K', 'M', 'B', 'D', 'H', 'V'};

// RNA alphabet (standard nucleotides)
constexpr char RNA_BASES[] = {'A', 'C', 'G', 'U'};
constexpr char RNA_BASES_EXTENDED[] = {'A', 'C', 'G', 'U', 'N', 'R', 'Y', 'S', 'W', 'K', 'M', 'B', 'D', 'H', 'V'};

// Protein alphabet (20 standard amino acids + stop)
constexpr char AMINO_ACIDS[] = {
    'A', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'K', 'L',
    'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'V', 'W', 'Y', '*'
};

// IUPAC ambiguity codes
// DNA/RNA:
// R = A or G (puRine)
// Y = C or T/U (pYrimidine)
// S = G or C (Strong)
// W = A or T/U (Weak)
// K = G or T/U (Keto)
// M = A or C (aMino)
// B = C or G or T/U (not A)
// D = A or G or T/U (not C)
// H = A or C or T/U (not G)
// V = A or C or G (not T/U)
// N = any base

class Alphabet {
public:
    // Check if character is valid for given sequence type
    static bool is_valid_char(char c, SequenceType type, bool allow_ambiguous = false);
    
    // Check if entire string is valid
    static bool is_valid_sequence(std::string_view seq, SequenceType type, bool allow_ambiguous = false);
    
    // Get complement of a single nucleotide
    static char complement(char base, SequenceType type);
    
    // Convert to uppercase
    static std::string to_upper(std::string_view seq);
    
    // Auto-detect sequence type
    static SequenceType detect_type(std::string_view seq);
    
    // Get standard genetic code (codon to amino acid)
    static const std::unordered_map<std::string, char>& get_genetic_code();
    
private:
    static const std::unordered_set<char> dna_standard_;
    static const std::unordered_set<char> dna_extended_;
    static const std::unordered_set<char> rna_standard_;
    static const std::unordered_set<char> rna_extended_;
    static const std::unordered_set<char> protein_standard_;
    static const std::unordered_map<char, char> dna_complement_;
    static const std::unordered_map<char, char> rna_complement_;
    static const std::unordered_map<std::string, char> genetic_code_;
};

} // namespace helixforge
