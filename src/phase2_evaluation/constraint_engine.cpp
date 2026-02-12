#include "constraint_engine.hpp"
#include <sstream>
#include <algorithm>
#include <regex>

namespace helixforge {

// ============================================================================
// ConstraintReport Implementation
// ============================================================================

void ConstraintReport::add_result(const ConstraintResult& result) {
    results_.push_back(result);
}

bool ConstraintReport::all_satisfied() const {
    return std::all_of(results_.begin(), results_.end(),
        [](const ConstraintResult& r) { return r.satisfied; });
}

std::vector<ConstraintResult> ConstraintReport::violations() const {
    std::vector<ConstraintResult> viols;
    std::copy_if(results_.begin(), results_.end(), std::back_inserter(viols),
        [](const ConstraintResult& r) { return !r.satisfied; });
    return viols;
}

size_t ConstraintReport::satisfied_count() const {
    return std::count_if(results_.begin(), results_.end(),
        [](const ConstraintResult& r) { return r.satisfied; });
}

size_t ConstraintReport::violated_count() const {
    return std::count_if(results_.begin(), results_.end(),
        [](const ConstraintResult& r) { return !r.satisfied; });
}

std::string ConstraintReport::to_string() const {
    std::ostringstream oss;
    oss << "Constraint Report:\n";
    oss << "------------------\n";
    oss << "Total: " << total_constraints() << " constraints\n";
    oss << "Satisfied: " << satisfied_count() << "\n";
    oss << "Violated: " << violated_count() << "\n\n";
    
    if (!all_satisfied()) {
        oss << "Violations:\n";
        for (const auto& result : results_) {
            if (!result.satisfied) {
                oss << "  [FAIL] " << result.constraint_name << ": " 
                    << result.violation_message << "\n";
                oss << "         Actual value: " << result.actual_value << "\n";
            }
        }
    } else {
        oss << "All constraints satisfied!\n";
    }
    
    return oss.str();
}

std::string ConstraintReport::to_json() const {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"all_satisfied\": " << (all_satisfied() ? "true" : "false") << ",\n";
    oss << "  \"total\": " << total_constraints() << ",\n";
    oss << "  \"satisfied\": " << satisfied_count() << ",\n";
    oss << "  \"violated\": " << violated_count() << ",\n";
    oss << "  \"results\": [\n";
    
    for (size_t i = 0; i < results_.size(); ++i) {
        const auto& result = results_[i];
        oss << "    {\n";
        oss << "      \"constraint\": \"" << result.constraint_name << "\",\n";
        oss << "      \"satisfied\": " << (result.satisfied ? "true" : "false") << ",\n";
        oss << "      \"actual_value\": " << result.actual_value;
        
        if (!result.violation_message.empty()) {
            oss << ",\n      \"message\": \"" << result.violation_message << "\"";
        }
        
        oss << "\n    }";
        if (i < results_.size() - 1) oss << ",";
        oss << "\n";
    }
    
    oss << "  ]\n";
    oss << "}";
    
    return oss.str();
}

// ============================================================================
// ConstraintEngine Implementation
// ============================================================================

void ConstraintEngine::add_constraint(std::shared_ptr<Constraint> constraint) {
    constraints_.push_back(constraint);
}

void ConstraintEngine::add_constraint_from_string(const std::string& constraint_str) {
    auto constraint = parse_constraint_string(constraint_str);
    if (constraint) {
        add_constraint(constraint);
    } else {
        throw std::invalid_argument("Unable to parse constraint: " + constraint_str);
    }
}

ConstraintReport ConstraintEngine::evaluate(const Sequence& seq) const {
    ConstraintReport report;
    
    for (const auto& constraint : constraints_) {
        try {
            ConstraintResult result = constraint->evaluate(seq);
            report.add_result(result);
        } catch (const std::exception& e) {
            report.add_result(ConstraintResult(
                constraint->name(), false, 0.0, std::string("Error: ") + e.what()
            ));
        }
    }
    
    return report;
}

bool ConstraintEngine::satisfies_all(const Sequence& seq) const {
    return evaluate(seq).all_satisfied();
}

void ConstraintEngine::clear() {
    constraints_.clear();
}

std::shared_ptr<Constraint> ConstraintEngine::parse_constraint_string(const std::string& constraint_str) {
    // Parse formats like:
    // "gc=0.45-0.60"
    // "length>100"
    // "avoid:GAATTC"
    // "require:ATG"
    // "homopolymer<5"
    
    // GC range: gc=min-max
    std::regex gc_range_regex(R"(gc\s*=\s*([\d.]+)\s*-\s*([\d.]+))");
    std::smatch match;
    if (std::regex_match(constraint_str, match, gc_range_regex)) {
        double min_gc = std::stod(match[1]);
        double max_gc = std::stod(match[2]);
        return std::make_shared<GCContentConstraint>(min_gc, max_gc);
    }
    
    // Length constraint: length>X or length<X or length=X-Y
    std::regex length_range_regex(R"(length\s*=\s*(\d+)\s*-\s*(\d+))");
    if (std::regex_match(constraint_str, match, length_range_regex)) {
        size_t min_len = std::stoull(match[1]);
        size_t max_len = std::stoull(match[2]);
        return std::make_shared<LengthConstraint>(min_len, max_len);
    }
    
    std::regex length_min_regex(R"(length\s*>\s*(\d+))");
    if (std::regex_match(constraint_str, match, length_min_regex)) {
        size_t min_len = std::stoull(match[1]);
        return std::make_shared<LengthConstraint>(min_len);
    }
    
    // Forbidden motif: avoid:MOTIF
    std::regex avoid_regex(R"(avoid\s*:\s*([A-Za-z]+))");
    if (std::regex_match(constraint_str, match, avoid_regex)) {
        std::string motif = match[1];
        return std::make_shared<ForbiddenMotifConstraint>(motif);
    }
    
    // Required motif: require:MOTIF
    std::regex require_regex(R"(require\s*:\s*([A-Za-z]+))");
    if (std::regex_match(constraint_str, match, require_regex)) {
        std::string motif = match[1];
        return std::make_shared<RequiredMotifConstraint>(motif);
    }
    
    // Homopolymer limit: homopolymer<X
    std::regex homopolymer_regex(R"(homopolymer\s*<\s*(\d+))");
    if (std::regex_match(constraint_str, match, homopolymer_regex)) {
        size_t max_len = std::stoull(match[1]);
        return std::make_shared<HomopolymerConstraint>(max_len);
    }
    
    // Valid ORF
    if (constraint_str == "valid_orf" || constraint_str == "orf") {
        return std::make_shared<ValidORFConstraint>();
    }
    
    // No premature stops
    if (constraint_str == "no_stop" || constraint_str == "no_stops") {
        return std::make_shared<NoPrematureStopConstraint>();
    }
    
    return nullptr;
}

// ============================================================================
// GCContentConstraint Implementation
// ============================================================================

GCContentConstraint::GCContentConstraint(double min_gc, double max_gc)
    : min_gc_(min_gc), max_gc_(max_gc) {}

ConstraintResult GCContentConstraint::evaluate(const Sequence& seq) const {
    double gc = seq.gc_content();
    bool satisfied = (gc >= min_gc_ && gc <= max_gc_);
    
    std::ostringstream msg;
    if (!satisfied) {
        msg << "GC content " << gc << " outside range [" << min_gc_ << ", " << max_gc_ << "]";
    }
    
    return ConstraintResult(name(), satisfied, gc, msg.str());
}

std::string GCContentConstraint::description() const {
    std::ostringstream oss;
    oss << "GC content must be between " << min_gc_ << " and " << max_gc_;
    return oss.str();
}

// ============================================================================
// LengthConstraint Implementation
// ============================================================================

LengthConstraint::LengthConstraint(size_t min_length, size_t max_length)
    : min_length_(min_length), max_length_(max_length) {}

ConstraintResult LengthConstraint::evaluate(const Sequence& seq) const {
    size_t len = seq.length();
    bool satisfied = (len >= min_length_ && len <= max_length_);
    
    std::ostringstream msg;
    if (!satisfied) {
        msg << "Length " << len << " outside range [" << min_length_ << ", " << max_length_ << "]";
    }
    
    return ConstraintResult(name(), satisfied, static_cast<double>(len), msg.str());
}

std::string LengthConstraint::description() const {
    std::ostringstream oss;
    oss << "Length must be between " << min_length_ << " and " << max_length_;
    return oss.str();
}

// ============================================================================
// ForbiddenMotifConstraint Implementation
// ============================================================================

ForbiddenMotifConstraint::ForbiddenMotifConstraint(std::string motif) {
    forbidden_motifs_.push_back(Alphabet::to_upper(std::move(motif)));
}

ForbiddenMotifConstraint::ForbiddenMotifConstraint(const std::vector<std::string>& motifs) {
    for (const auto& motif : motifs) {
        forbidden_motifs_.push_back(Alphabet::to_upper(motif));
    }
}

void ForbiddenMotifConstraint::add_motif(const std::string& motif) {
    forbidden_motifs_.push_back(Alphabet::to_upper(motif));
}

ConstraintResult ForbiddenMotifConstraint::evaluate(const Sequence& seq) const {
    for (const auto& motif : forbidden_motifs_) {
        size_t count = seq.count(motif);
        if (count > 0) {
            std::ostringstream msg;
            msg << "Found forbidden motif '" << motif << "' (" << count << " occurrences)";
            return ConstraintResult(name(), false, static_cast<double>(count), msg.str());
        }
    }
    
    return ConstraintResult(name(), true, 0.0);
}

std::string ForbiddenMotifConstraint::description() const {
    std::ostringstream oss;
    oss << "Sequence must not contain: ";
    for (size_t i = 0; i < forbidden_motifs_.size(); ++i) {
        oss << forbidden_motifs_[i];
        if (i < forbidden_motifs_.size() - 1) oss << ", ";
    }
    return oss.str();
}

// ============================================================================
// RequiredMotifConstraint Implementation
// ============================================================================

RequiredMotifConstraint::RequiredMotifConstraint(std::string motif) {
    required_motifs_.push_back(Alphabet::to_upper(std::move(motif)));
}

RequiredMotifConstraint::RequiredMotifConstraint(const std::vector<std::string>& motifs) {
    for (const auto& motif : motifs) {
        required_motifs_.push_back(Alphabet::to_upper(motif));
    }
}

void RequiredMotifConstraint::add_motif(const std::string& motif) {
    required_motifs_.push_back(Alphabet::to_upper(motif));
}

ConstraintResult RequiredMotifConstraint::evaluate(const Sequence& seq) const {
    for (const auto& motif : required_motifs_) {
        size_t count = seq.count(motif);
        if (count == 0) {
            std::ostringstream msg;
            msg << "Missing required motif '" << motif << "'";
            return ConstraintResult(name(), false, 0.0, msg.str());
        }
    }
    
    return ConstraintResult(name(), true, 1.0);
}

std::string RequiredMotifConstraint::description() const {
    std::ostringstream oss;
    oss << "Sequence must contain: ";
    for (size_t i = 0; i < required_motifs_.size(); ++i) {
        oss << required_motifs_[i];
        if (i < required_motifs_.size() - 1) oss << ", ";
    }
    return oss.str();
}

// ============================================================================
// HomopolymerConstraint Implementation
// ============================================================================

HomopolymerConstraint::HomopolymerConstraint(size_t max_length)
    : max_length_(max_length) {}

ConstraintResult HomopolymerConstraint::evaluate(const Sequence& seq) const {
    if (seq.empty()) {
        return ConstraintResult(name(), true, 0.0);
    }
    
    size_t max_run = 1;
    size_t current_run = 1;
    char current_char = seq[0];
    
    for (size_t i = 1; i < seq.length(); ++i) {
        if (seq[i] == current_char) {
            ++current_run;
            max_run = std::max(max_run, current_run);
        } else {
            current_run = 1;
            current_char = seq[i];
        }
    }
    
    bool satisfied = (max_run <= max_length_);
    
    std::ostringstream msg;
    if (!satisfied) {
        msg << "Homopolymer run of length " << max_run << " exceeds limit " << max_length_;
    }
    
    return ConstraintResult(name(), satisfied, static_cast<double>(max_run), msg.str());
}

std::string HomopolymerConstraint::description() const {
    std::ostringstream oss;
    oss << "Homopolymer runs must not exceed " << max_length_ << " bases";
    return oss.str();
}

// ============================================================================
// ValidORFConstraint Implementation
// ============================================================================

ValidORFConstraint::ValidORFConstraint(size_t min_length, bool require_start_codon)
    : min_length_(min_length), require_start_codon_(require_start_codon) {}

ConstraintResult ValidORFConstraint::evaluate(const Sequence& seq) const {
    auto orfs = seq.find_orfs(min_length_);
    
    bool satisfied = !orfs.empty();
    
    std::ostringstream msg;
    if (!satisfied) {
        msg << "No valid ORF of minimum length " << min_length_ << " found";
    }
    
    return ConstraintResult(name(), satisfied, static_cast<double>(orfs.size()), msg.str());
}

std::string ValidORFConstraint::description() const {
    std::ostringstream oss;
    oss << "Sequence must contain a valid ORF of at least " << min_length_ << " bases";
    return oss.str();
}

// ============================================================================
// NoPrematureStopConstraint Implementation
// ============================================================================

ConstraintResult NoPrematureStopConstraint::evaluate(const Sequence& seq) const {
    if (seq.type() != SequenceType::DNA && seq.type() != SequenceType::RNA) {
        return ConstraintResult(name(), true, 0.0, "Not applicable to protein sequences");
    }
    
    try {
        // Translate without stopping at stop codons
        Sequence protein = seq.translate(0, false);
        
        // Count stop codons in the middle (not at the end)
        size_t stop_count = 0;
        const std::string& protein_data = protein.data();
        
        // Check all but the last amino acid
        for (size_t i = 0; i < protein_data.length() - 1; ++i) {
            if (protein_data[i] == '*') {
                ++stop_count;
            }
        }
        
        bool satisfied = (stop_count == 0);
        
        std::ostringstream msg;
        if (!satisfied) {
            msg << "Found " << stop_count << " premature stop codon(s)";
        }
        
        return ConstraintResult(name(), satisfied, static_cast<double>(stop_count), msg.str());
        
    } catch (const std::exception& e) {
        return ConstraintResult(name(), false, 0.0, std::string("Translation error: ") + e.what());
    }
}

std::string NoPrematureStopConstraint::description() const {
    return "Sequence must not contain premature stop codons";
}

// ============================================================================
// CustomConstraint Implementation
// ============================================================================

CustomConstraint::CustomConstraint(std::string name, EvaluatorFunc func, std::string description)
    : name_(std::move(name)), description_(std::move(description)), evaluator_(std::move(func)) {}

ConstraintResult CustomConstraint::evaluate(const Sequence& seq) const {
    try {
        bool satisfied = evaluator_(seq);
        return ConstraintResult(name_, satisfied, satisfied ? 1.0 : 0.0);
    } catch (const std::exception& e) {
        return ConstraintResult(name_, false, 0.0, std::string("Error: ") + e.what());
    }
}

} // namespace helixforge
