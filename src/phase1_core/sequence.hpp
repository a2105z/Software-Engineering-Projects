#pragma once

#include "types.hpp"
#include "alphabet.hpp"
#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <iosfwd>

namespace helixforge {

/**
 * Core sequence class representing DNA, RNA, or protein sequences
 * Provides validation, manipulation, and biological operations
 */
class Sequence {
public:
    // Constructors
    Sequence();
    Sequence(std::string data, SequenceType type, std::string id = "", std::string description = "");
    
    // Factory methods for explicit type construction
    static Sequence from_dna(std::string data, std::string id = "", std::string description = "");
    static Sequence from_rna(std::string data, std::string id = "", std::string description = "");
    static Sequence from_protein(std::string data, std::string id = "", std::string description = "");
    
    // Parse from FASTA format
    static std::vector<Sequence> from_fasta(std::string_view fasta_content);
    static Sequence from_fasta_single(std::string_view fasta_content);
    
    // Accessors
    const std::string& data() const { return data_; }
    std::string& data() { return data_; }
    SequenceType type() const { return type_; }
    const std::string& id() const { return id_; }
    const std::string& description() const { return description_; }
    size_t length() const { return data_.length(); }
    bool empty() const { return data_.empty(); }
    
    // Mutators
    void set_id(std::string id) { id_ = std::move(id); }
    void set_description(std::string desc) { description_ = std::move(desc); }
    void set_type(SequenceType type);
    
    // Element access
    char operator[](size_t pos) const { return data_[pos]; }
    char& operator[](size_t pos) { return data_[pos]; }
    char at(size_t pos) const;
    
    // Iterators
    auto begin() { return data_.begin(); }
    auto end() { return data_.end(); }
    auto begin() const { return data_.begin(); }
    auto end() const { return data_.end(); }
    
    // Validation
    bool validate(bool allow_ambiguous = false) const;
    void validate_or_throw(bool allow_ambiguous = false) const;
    
    // Subsequence operations
    Sequence subseq(Position start, Length length) const;
    Sequence slice(Position start, Position end) const;
    
    // Biological operations
    Sequence reverse() const;
    Sequence complement() const;
    Sequence reverse_complement() const;
    
    // Transcription (DNA -> RNA)
    Sequence transcribe() const;
    
    // Reverse transcription (RNA -> DNA)
    Sequence reverse_transcribe() const;
    
    // Translation (DNA/RNA -> Protein)
    Sequence translate(int frame = 0, bool to_stop = false) const;
    
    // Find all open reading frames (returns start positions)
    std::vector<Position> find_orfs(size_t min_length = 100) const;
    
    // GC content calculation
    double gc_content() const;
    
    // Count specific character/pattern
    size_t count(char c) const;
    size_t count(std::string_view pattern) const;
    
    // Find motif positions
    std::vector<Position> find_motif(std::string_view motif) const;
    
    // Output to FASTA format
    std::string to_fasta(size_t line_width = 80) const;
    
    // Comparison operators
    bool operator==(const Sequence& other) const;
    bool operator!=(const Sequence& other) const { return !(*this == other); }
    
    // String conversion
    std::string to_string() const;
    
private:
    std::string data_;
    SequenceType type_;
    std::string id_;
    std::string description_;
    
    // Helper for translation
    static char translate_codon(std::string_view codon);
    
    // Helper for parsing FASTA
    static void trim(std::string& s);
};

// Stream operators
std::ostream& operator<<(std::ostream& os, const Sequence& seq);

} // namespace helixforge
