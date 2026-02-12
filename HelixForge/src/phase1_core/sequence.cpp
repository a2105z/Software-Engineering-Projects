#include "sequence.hpp"
#include <algorithm>
#include <sstream>
#include <iostream>
#include <cctype>

namespace helixforge {

// ============================================================================
// Alphabet Implementation
// ============================================================================

const std::unordered_set<char> Alphabet::dna_standard_ = {'A', 'C', 'G', 'T'};
const std::unordered_set<char> Alphabet::dna_extended_ = {'A', 'C', 'G', 'T', 'N', 'R', 'Y', 'S', 'W', 'K', 'M', 'B', 'D', 'H', 'V'};
const std::unordered_set<char> Alphabet::rna_standard_ = {'A', 'C', 'G', 'U'};
const std::unordered_set<char> Alphabet::rna_extended_ = {'A', 'C', 'G', 'U', 'N', 'R', 'Y', 'S', 'W', 'K', 'M', 'B', 'D', 'H', 'V'};
const std::unordered_set<char> Alphabet::protein_standard_ = {
    'A', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'K', 'L',
    'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'V', 'W', 'Y', '*'
};

const std::unordered_map<char, char> Alphabet::dna_complement_ = {
    {'A', 'T'}, {'T', 'A'}, {'C', 'G'}, {'G', 'C'},
    {'N', 'N'}, {'R', 'Y'}, {'Y', 'R'}, {'S', 'S'},
    {'W', 'W'}, {'K', 'M'}, {'M', 'K'}, {'B', 'V'},
    {'D', 'H'}, {'H', 'D'}, {'V', 'B'}
};

const std::unordered_map<char, char> Alphabet::rna_complement_ = {
    {'A', 'U'}, {'U', 'A'}, {'C', 'G'}, {'G', 'C'},
    {'N', 'N'}, {'R', 'Y'}, {'Y', 'R'}, {'S', 'S'},
    {'W', 'W'}, {'K', 'M'}, {'M', 'K'}, {'B', 'V'},
    {'D', 'H'}, {'H', 'D'}, {'V', 'B'}
};

// Standard genetic code (DNA codons)
const std::unordered_map<std::string, char> Alphabet::genetic_code_ = {
    // Phenylalanine
    {"TTT", 'F'}, {"TTC", 'F'},
    // Leucine
    {"TTA", 'L'}, {"TTG", 'L'}, {"CTT", 'L'}, {"CTC", 'L'}, {"CTA", 'L'}, {"CTG", 'L'},
    // Isoleucine
    {"ATT", 'I'}, {"ATC", 'I'}, {"ATA", 'I'},
    // Methionine (start)
    {"ATG", 'M'},
    // Valine
    {"GTT", 'V'}, {"GTC", 'V'}, {"GTA", 'V'}, {"GTG", 'V'},
    // Serine
    {"TCT", 'S'}, {"TCC", 'S'}, {"TCA", 'S'}, {"TCG", 'S'}, {"AGT", 'S'}, {"AGC", 'S'},
    // Proline
    {"CCT", 'P'}, {"CCC", 'P'}, {"CCA", 'P'}, {"CCG", 'P'},
    // Threonine
    {"ACT", 'T'}, {"ACC", 'T'}, {"ACA", 'T'}, {"ACG", 'T'},
    // Alanine
    {"GCT", 'A'}, {"GCC", 'A'}, {"GCA", 'A'}, {"GCG", 'A'},
    // Tyrosine
    {"TAT", 'Y'}, {"TAC", 'Y'},
    // Stop codons
    {"TAA", '*'}, {"TAG", '*'}, {"TGA", '*'},
    // Histidine
    {"CAT", 'H'}, {"CAC", 'H'},
    // Glutamine
    {"CAA", 'Q'}, {"CAG", 'Q'},
    // Asparagine
    {"AAT", 'N'}, {"AAC", 'N'},
    // Lysine
    {"AAA", 'K'}, {"AAG", 'K'},
    // Aspartic acid
    {"GAT", 'D'}, {"GAC", 'D'},
    // Glutamic acid
    {"GAA", 'E'}, {"GAG", 'E'},
    // Cysteine
    {"TGT", 'C'}, {"TGC", 'C'},
    // Tryptophan
    {"TGG", 'W'},
    // Arginine
    {"CGT", 'R'}, {"CGC", 'R'}, {"CGA", 'R'}, {"CGG", 'R'}, {"AGA", 'R'}, {"AGG", 'R'},
    // Glycine
    {"GGT", 'G'}, {"GGC", 'G'}, {"GGA", 'G'}, {"GGG", 'G'}
};

bool Alphabet::is_valid_char(char c, SequenceType type, bool allow_ambiguous) {
    c = std::toupper(c);
    
    switch (type) {
        case SequenceType::DNA:
            return allow_ambiguous ? 
                dna_extended_.count(c) > 0 : dna_standard_.count(c) > 0;
        case SequenceType::RNA:
            return allow_ambiguous ? 
                rna_extended_.count(c) > 0 : rna_standard_.count(c) > 0;
        case SequenceType::PROTEIN:
            return protein_standard_.count(c) > 0;
        default:
            return false;
    }
}

bool Alphabet::is_valid_sequence(std::string_view seq, SequenceType type, bool allow_ambiguous) {
    return std::all_of(seq.begin(), seq.end(), 
        [type, allow_ambiguous](char c) {
            return is_valid_char(c, type, allow_ambiguous);
        });
}

char Alphabet::complement(char base, SequenceType type) {
    base = std::toupper(base);
    
    if (type == SequenceType::DNA) {
        auto it = dna_complement_.find(base);
        if (it != dna_complement_.end()) {
            return it->second;
        }
    } else if (type == SequenceType::RNA) {
        auto it = rna_complement_.find(base);
        if (it != rna_complement_.end()) {
            return it->second;
        }
    }
    
    throw ValidationException(std::string("Cannot complement base: ") + base);
}

std::string Alphabet::to_upper(std::string_view seq) {
    std::string result;
    result.reserve(seq.length());
    std::transform(seq.begin(), seq.end(), std::back_inserter(result), 
        [](char c) { return std::toupper(c); });
    return result;
}

SequenceType Alphabet::detect_type(std::string_view seq) {
    if (seq.empty()) {
        return SequenceType::UNKNOWN;
    }
    
    bool has_t = false;
    bool has_u = false;
    bool has_protein_specific = false;
    
    for (char c : seq) {
        c = std::toupper(c);
        if (c == 'T') has_t = true;
        if (c == 'U') has_u = true;
        if (protein_standard_.count(c) > 0 && 
            dna_standard_.count(c) == 0 && 
            rna_standard_.count(c) == 0) {
            has_protein_specific = true;
        }
    }
    
    if (has_protein_specific) {
        return SequenceType::PROTEIN;
    }
    if (has_u && !has_t) {
        return SequenceType::RNA;
    }
    if (has_t && !has_u) {
        return SequenceType::DNA;
    }
    
    // Default to DNA if only ACGN
    if (is_valid_sequence(seq, SequenceType::DNA, true)) {
        return SequenceType::DNA;
    }
    
    return SequenceType::UNKNOWN;
}

const std::unordered_map<std::string, char>& Alphabet::get_genetic_code() {
    return genetic_code_;
}

// ============================================================================
// Sequence Implementation
// ============================================================================

Sequence::Sequence() 
    : data_(), type_(SequenceType::UNKNOWN), id_(), description_() {}

Sequence::Sequence(std::string data, SequenceType type, std::string id, std::string description)
    : data_(Alphabet::to_upper(data)), type_(type), id_(std::move(id)), description_(std::move(description)) {
    
    if (type_ == SequenceType::UNKNOWN) {
        type_ = Alphabet::detect_type(data_);
    }
}

Sequence Sequence::from_dna(std::string data, std::string id, std::string description) {
    Sequence seq(std::move(data), SequenceType::DNA, std::move(id), std::move(description));
    seq.validate_or_throw(true);
    return seq;
}

Sequence Sequence::from_rna(std::string data, std::string id, std::string description) {
    Sequence seq(std::move(data), SequenceType::RNA, std::move(id), std::move(description));
    seq.validate_or_throw(true);
    return seq;
}

Sequence Sequence::from_protein(std::string data, std::string id, std::string description) {
    Sequence seq(std::move(data), SequenceType::PROTEIN, std::move(id), std::move(description));
    seq.validate_or_throw(true);
    return seq;
}

void Sequence::trim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

std::vector<Sequence> Sequence::from_fasta(std::string_view fasta_content) {
    std::vector<Sequence> sequences;
    std::istringstream stream(std::string(fasta_content));
    std::string line;
    std::string current_id;
    std::string current_desc;
    std::string current_data;
    
    while (std::getline(stream, line)) {
        trim(line);
        if (line.empty()) continue;
        
        if (line[0] == '>') {
            // Save previous sequence if exists
            if (!current_data.empty()) {
                SequenceType type = Alphabet::detect_type(current_data);
                sequences.emplace_back(current_data, type, current_id, current_desc);
            }
            
            // Parse header
            line = line.substr(1); // Remove '>'
            size_t space_pos = line.find(' ');
            if (space_pos != std::string::npos) {
                current_id = line.substr(0, space_pos);
                current_desc = line.substr(space_pos + 1);
            } else {
                current_id = line;
                current_desc = "";
            }
            trim(current_id);
            trim(current_desc);
            current_data.clear();
        } else {
            current_data += line;
        }
    }
    
    // Don't forget last sequence
    if (!current_data.empty()) {
        SequenceType type = Alphabet::detect_type(current_data);
        sequences.emplace_back(current_data, type, current_id, current_desc);
    }
    
    return sequences;
}

Sequence Sequence::from_fasta_single(std::string_view fasta_content) {
    auto sequences = from_fasta(fasta_content);
    if (sequences.empty()) {
        throw SequenceException("No sequences found in FASTA content");
    }
    if (sequences.size() > 1) {
        throw SequenceException("Multiple sequences found, expected single sequence");
    }
    return sequences[0];
}

void Sequence::set_type(SequenceType type) {
    type_ = type;
    validate_or_throw(true);
}

char Sequence::at(size_t pos) const {
    if (pos >= data_.length()) {
        throw std::out_of_range("Sequence position out of range");
    }
    return data_[pos];
}

bool Sequence::validate(bool allow_ambiguous) const {
    return Alphabet::is_valid_sequence(data_, type_, allow_ambiguous);
}

void Sequence::validate_or_throw(bool allow_ambiguous) const {
    if (!validate(allow_ambiguous)) {
        throw ValidationException(
            "Invalid sequence data for type " + sequence_type_to_string(type_)
        );
    }
}

Sequence Sequence::subseq(Position start, Length length) const {
    if (start >= data_.length()) {
        throw std::out_of_range("Start position out of range");
    }
    
    Length actual_length = std::min(length, data_.length() - start);
    std::string sub_data = data_.substr(start, actual_length);
    
    return Sequence(sub_data, type_, id_ + "_subseq", description_);
}

Sequence Sequence::slice(Position start, Position end) const {
    if (start >= data_.length() || end > data_.length() || start >= end) {
        throw std::out_of_range("Invalid slice range");
    }
    
    return subseq(start, end - start);
}

Sequence Sequence::reverse() const {
    std::string reversed_data = data_;
    std::reverse(reversed_data.begin(), reversed_data.end());
    return Sequence(reversed_data, type_, id_ + "_rev", description_);
}

Sequence Sequence::complement() const {
    if (type_ != SequenceType::DNA && type_ != SequenceType::RNA) {
        throw SequenceException("Complement only valid for DNA/RNA sequences");
    }
    
    std::string comp_data;
    comp_data.reserve(data_.length());
    
    for (char base : data_) {
        comp_data += Alphabet::complement(base, type_);
    }
    
    return Sequence(comp_data, type_, id_ + "_comp", description_);
}

Sequence Sequence::reverse_complement() const {
    return complement().reverse();
}

Sequence Sequence::transcribe() const {
    if (type_ != SequenceType::DNA) {
        throw SequenceException("Transcription only valid for DNA sequences");
    }
    
    std::string rna_data = data_;
    std::replace(rna_data.begin(), rna_data.end(), 'T', 'U');
    
    return Sequence(rna_data, SequenceType::RNA, id_ + "_rna", description_);
}

Sequence Sequence::reverse_transcribe() const {
    if (type_ != SequenceType::RNA) {
        throw SequenceException("Reverse transcription only valid for RNA sequences");
    }
    
    std::string dna_data = data_;
    std::replace(dna_data.begin(), dna_data.end(), 'U', 'T');
    
    return Sequence(dna_data, SequenceType::DNA, id_ + "_dna", description_);
}

char Sequence::translate_codon(std::string_view codon) {
    std::string codon_str(codon);
    
    // Convert RNA to DNA for lookup
    std::replace(codon_str.begin(), codon_str.end(), 'U', 'T');
    
    const auto& code = Alphabet::get_genetic_code();
    auto it = code.find(codon_str);
    
    if (it != code.end()) {
        return it->second;
    }
    
    // Unknown codon
    return 'X';
}

Sequence Sequence::translate(int frame, bool to_stop) const {
    if (type_ != SequenceType::DNA && type_ != SequenceType::RNA) {
        throw TranslationException("Translation only valid for DNA/RNA sequences");
    }
    
    if (frame < 0 || frame > 2) {
        throw TranslationException("Reading frame must be 0, 1, or 2");
    }
    
    std::string protein_data;
    
    for (size_t i = frame; i + CODON_LENGTH <= data_.length(); i += CODON_LENGTH) {
        std::string codon = data_.substr(i, CODON_LENGTH);
        char aa = translate_codon(codon);
        
        if (to_stop && aa == '*') {
            break;
        }
        
        protein_data += aa;
    }
    
    return Sequence(protein_data, SequenceType::PROTEIN, id_ + "_protein", description_);
}

std::vector<Position> Sequence::find_orfs(size_t min_length) const {
    if (type_ != SequenceType::DNA && type_ != SequenceType::RNA) {
        throw SequenceException("ORF finding only valid for DNA/RNA sequences");
    }
    
    std::vector<Position> orf_starts;
    const std::string start_codon = (type_ == SequenceType::DNA) ? "ATG" : "AUG";
    const std::vector<std::string> stop_codons = 
        (type_ == SequenceType::DNA) ? 
        std::vector<std::string>{"TAA", "TAG", "TGA"} :
        std::vector<std::string>{"UAA", "UAG", "UGA"};
    
    // Check all three reading frames
    for (int frame = 0; frame < 3; ++frame) {
        for (size_t i = frame; i + CODON_LENGTH <= data_.length(); i += CODON_LENGTH) {
            std::string codon = data_.substr(i, CODON_LENGTH);
            
            if (codon == start_codon) {
                // Found start codon, look for stop
                size_t orf_length = 0;
                bool found_stop = false;
                
                for (size_t j = i; j + CODON_LENGTH <= data_.length(); j += CODON_LENGTH) {
                    std::string current_codon = data_.substr(j, CODON_LENGTH);
                    orf_length += CODON_LENGTH;
                    
                    for (const auto& stop : stop_codons) {
                        if (current_codon == stop) {
                            found_stop = true;
                            break;
                        }
                    }
                    
                    if (found_stop) break;
                }
                
                if (orf_length >= min_length) {
                    orf_starts.push_back(i);
                }
            }
        }
    }
    
    return orf_starts;
}

double Sequence::gc_content() const {
    if (type_ != SequenceType::DNA && type_ != SequenceType::RNA) {
        return 0.0;
    }
    
    if (data_.empty()) {
        return 0.0;
    }
    
    size_t gc_count = 0;
    for (char base : data_) {
        if (base == 'G' || base == 'C') {
            ++gc_count;
        }
    }
    
    return static_cast<double>(gc_count) / static_cast<double>(data_.length());
}

size_t Sequence::count(char c) const {
    c = std::toupper(c);
    return std::count(data_.begin(), data_.end(), c);
}

size_t Sequence::count(std::string_view pattern) const {
    if (pattern.empty() || pattern.length() > data_.length()) {
        return 0;
    }
    
    size_t count = 0;
    size_t pos = 0;
    
    while ((pos = data_.find(pattern, pos)) != std::string::npos) {
        ++count;
        ++pos;
    }
    
    return count;
}

std::vector<Position> Sequence::find_motif(std::string_view motif) const {
    std::vector<Position> positions;
    
    if (motif.empty() || motif.length() > data_.length()) {
        return positions;
    }
    
    std::string motif_upper = Alphabet::to_upper(motif);
    size_t pos = 0;
    
    while ((pos = data_.find(motif_upper, pos)) != std::string::npos) {
        positions.push_back(pos);
        ++pos;
    }
    
    return positions;
}

std::string Sequence::to_fasta(size_t line_width) const {
    std::ostringstream oss;
    
    // Header line
    oss << ">" << id_;
    if (!description_.empty()) {
        oss << " " << description_;
    }
    oss << "\n";
    
    // Sequence data with line wrapping
    for (size_t i = 0; i < data_.length(); i += line_width) {
        size_t chunk_size = std::min(line_width, data_.length() - i);
        oss << data_.substr(i, chunk_size) << "\n";
    }
    
    return oss.str();
}

bool Sequence::operator==(const Sequence& other) const {
    return data_ == other.data_ && type_ == other.type_;
}

std::string Sequence::to_string() const {
    return data_;
}

std::ostream& operator<<(std::ostream& os, const Sequence& seq) {
    os << seq.to_fasta();
    return os;
}

} // namespace helixforge
