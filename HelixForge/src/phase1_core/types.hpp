#pragma once

#include <cstdint>
#include <string>
#include <stdexcept>

namespace helixforge {

// Sequence type classification
enum class SequenceType {
    DNA,
    RNA,
    PROTEIN,
    UNKNOWN
};

// Exception types for sequence operations
class SequenceException : public std::runtime_error {
public:
    explicit SequenceException(const std::string& msg) 
        : std::runtime_error(msg) {}
};

class ValidationException : public SequenceException {
public:
    explicit ValidationException(const std::string& msg) 
        : SequenceException("Validation error: " + msg) {}
};

class TranslationException : public SequenceException {
public:
    explicit TranslationException(const std::string& msg) 
        : SequenceException("Translation error: " + msg) {}
};

// Type aliases
using SequenceData = std::string;
using Position = size_t;
using Length = size_t;

// Constants
constexpr int CODON_LENGTH = 3;
constexpr char GAP_CHAR = '-';
constexpr char UNKNOWN_CHAR = 'N';

// Utility function to convert SequenceType to string
inline std::string sequence_type_to_string(SequenceType type) {
    switch (type) {
        case SequenceType::DNA: return "DNA";
        case SequenceType::RNA: return "RNA";
        case SequenceType::PROTEIN: return "PROTEIN";
        case SequenceType::UNKNOWN: return "UNKNOWN";
        default: return "INVALID";
    }
}

} // namespace helixforge
