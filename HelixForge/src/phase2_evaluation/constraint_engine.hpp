#pragma once

#include "../phase1_core/sequence.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <optional>
#include <limits>

namespace helixforge {

/**
 * Result of a constraint evaluation
 */
struct ConstraintResult {
    std::string constraint_name;
    bool satisfied;
    std::string violation_message;
    double actual_value;
    
    ConstraintResult(std::string name, bool sat, double val = 0.0, std::string msg = "")
        : constraint_name(std::move(name))
        , satisfied(sat)
        , violation_message(std::move(msg))
        , actual_value(val) {}
};

/**
 * Collection of constraint evaluation results
 */
class ConstraintReport {
public:
    ConstraintReport() = default;
    
    void add_result(const ConstraintResult& result);
    
    // Check if all constraints are satisfied
    bool all_satisfied() const;
    
    // Get list of violations
    std::vector<ConstraintResult> violations() const;
    
    // Get all results
    const std::vector<ConstraintResult>& results() const { return results_; }
    
    // Summary
    size_t total_constraints() const { return results_.size(); }
    size_t satisfied_count() const;
    size_t violated_count() const;
    
    // Export
    std::string to_string() const;
    std::string to_json() const;
    
private:
    std::vector<ConstraintResult> results_;
};

/**
 * Abstract base class for constraints
 */
class Constraint {
public:
    virtual ~Constraint() = default;
    
    // Check if constraint is satisfied
    virtual ConstraintResult evaluate(const Sequence& seq) const = 0;
    
    // Get constraint name
    virtual std::string name() const = 0;
    
    // Get description
    virtual std::string description() const = 0;
};

/**
 * Constraint engine - manages and evaluates multiple constraints
 */
class ConstraintEngine {
public:
    ConstraintEngine() = default;
    
    // Register a constraint
    void add_constraint(std::shared_ptr<Constraint> constraint);
    
    // Parse and add constraint from string (e.g., "gc=0.45-0.60")
    void add_constraint_from_string(const std::string& constraint_str);
    
    // Evaluate all constraints
    ConstraintReport evaluate(const Sequence& seq) const;
    
    // Check if all constraints are satisfied
    bool satisfies_all(const Sequence& seq) const;
    
    // Clear all constraints
    void clear();
    
    // Get number of constraints
    size_t count() const { return constraints_.size(); }
    
private:
    std::vector<std::shared_ptr<Constraint>> constraints_;
    
    // Factory for built-in constraints
    static std::shared_ptr<Constraint> parse_constraint_string(const std::string& constraint_str);
};

// ============================================================================
// Built-in Constraint Types
// ============================================================================

/**
 * GC content constraint (range)
 */
class GCContentConstraint : public Constraint {
public:
    GCContentConstraint(double min_gc, double max_gc);
    
    ConstraintResult evaluate(const Sequence& seq) const override;
    std::string name() const override { return "gc_content_range"; }
    std::string description() const override;
    
private:
    double min_gc_;
    double max_gc_;
};

/**
 * Length constraint (min/max)
 */
class LengthConstraint : public Constraint {
public:
    LengthConstraint(size_t min_length, size_t max_length = std::numeric_limits<size_t>::max());
    
    ConstraintResult evaluate(const Sequence& seq) const override;
    std::string name() const override { return "length_constraint"; }
    std::string description() const override;
    
private:
    size_t min_length_;
    size_t max_length_;
};

/**
 * Forbidden motif constraint (sequence must not contain motif)
 */
class ForbiddenMotifConstraint : public Constraint {
public:
    explicit ForbiddenMotifConstraint(std::string motif);
    ForbiddenMotifConstraint(const std::vector<std::string>& motifs);
    
    ConstraintResult evaluate(const Sequence& seq) const override;
    std::string name() const override { return "forbidden_motif"; }
    std::string description() const override;
    
    void add_motif(const std::string& motif);
    
private:
    std::vector<std::string> forbidden_motifs_;
};

/**
 * Required motif constraint (sequence must contain motif)
 */
class RequiredMotifConstraint : public Constraint {
public:
    explicit RequiredMotifConstraint(std::string motif);
    RequiredMotifConstraint(const std::vector<std::string>& motifs);
    
    ConstraintResult evaluate(const Sequence& seq) const override;
    std::string name() const override { return "required_motif"; }
    std::string description() const override;
    
    void add_motif(const std::string& motif);
    
private:
    std::vector<std::string> required_motifs_;
};

/**
 * Homopolymer constraint (limit consecutive repeats)
 */
class HomopolymerConstraint : public Constraint {
public:
    explicit HomopolymerConstraint(size_t max_length);
    
    ConstraintResult evaluate(const Sequence& seq) const override;
    std::string name() const override { return "homopolymer_limit"; }
    std::string description() const override;
    
private:
    size_t max_length_;
};

/**
 * Valid ORF constraint (must have valid open reading frame)
 */
class ValidORFConstraint : public Constraint {
public:
    explicit ValidORFConstraint(size_t min_length = 100, bool require_start_codon = true);
    
    ConstraintResult evaluate(const Sequence& seq) const override;
    std::string name() const override { return "valid_orf"; }
    std::string description() const override;
    
private:
    size_t min_length_;
    bool require_start_codon_;
};

/**
 * No premature stop codons constraint
 */
class NoPrematureStopConstraint : public Constraint {
public:
    NoPrematureStopConstraint() = default;
    
    ConstraintResult evaluate(const Sequence& seq) const override;
    std::string name() const override { return "no_premature_stops"; }
    std::string description() const override;
};

/**
 * Custom function constraint (user-defined lambda)
 */
class CustomConstraint : public Constraint {
public:
    using EvaluatorFunc = std::function<bool(const Sequence&)>;
    
    CustomConstraint(std::string name, EvaluatorFunc func, std::string description = "");
    
    ConstraintResult evaluate(const Sequence& seq) const override;
    std::string name() const override { return name_; }
    std::string description() const override { return description_; }
    
private:
    std::string name_;
    std::string description_;
    EvaluatorFunc evaluator_;
};

} // namespace helixforge
