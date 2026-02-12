#pragma once

#include "scoring.hpp"
#include <unordered_map>
#include <set>

namespace helixforge {

// ============================================================================
// GC Content Metric
// ============================================================================

class GCContentMetric : public ScoreMetric {
public:
    ScoreResult evaluate(const Sequence& seq) const override;
    std::string name() const override { return metrics::GC_CONTENT; }
    std::string description() const override { return "GC content ratio (0-1)"; }
    bool is_applicable(SequenceType type) const override;
};

// ============================================================================
// Codon Adaptation Index (CAI)
// Measures codon usage bias relative to highly expressed genes
// ============================================================================

class CodonAdaptationIndexMetric : public ScoreMetric {
public:
    CodonAdaptationIndexMetric();
    explicit CodonAdaptationIndexMetric(const std::unordered_map<std::string, double>& custom_weights);
    
    ScoreResult evaluate(const Sequence& seq) const override;
    std::string name() const override { return metrics::CODON_ADAPTATION_INDEX; }
    std::string description() const override { return "Codon Adaptation Index (0-1, higher is better)"; }
    bool is_applicable(SequenceType type) const override;
    
private:
    std::unordered_map<std::string, double> codon_weights_;
    void initialize_default_weights();
};

// ============================================================================
// Codon Usage Bias
// General codon usage deviation from uniform distribution
// ============================================================================

class CodonUsageBiasMetric : public ScoreMetric {
public:
    ScoreResult evaluate(const Sequence& seq) const override;
    std::string name() const override { return metrics::CODON_USAGE_BIAS; }
    std::string description() const override { return "Codon usage bias score"; }
    bool is_applicable(SequenceType type) const override;
};

// ============================================================================
// ORF Integrity
// Checks for premature stop codons and valid ORF structure
// ============================================================================

class ORFIntegrityMetric : public ScoreMetric {
public:
    explicit ORFIntegrityMetric(size_t min_orf_length = 100);
    
    ScoreResult evaluate(const Sequence& seq) const override;
    std::string name() const override { return metrics::ORF_INTEGRITY; }
    std::string description() const override { return "Open reading frame integrity score"; }
    bool is_applicable(SequenceType type) const override;
    
private:
    size_t min_orf_length_;
};

// ============================================================================
// Restriction Sites
// Identifies common restriction enzyme recognition sites
// ============================================================================

class RestrictionSitesMetric : public ScoreMetric {
public:
    RestrictionSitesMetric();
    explicit RestrictionSitesMetric(const std::vector<std::string>& custom_sites);
    
    ScoreResult evaluate(const Sequence& seq) const override;
    std::string name() const override { return metrics::RESTRICTION_SITES; }
    std::string description() const override { return "Number of restriction enzyme sites found"; }
    bool is_applicable(SequenceType type) const override;
    
    void add_site(const std::string& enzyme_name, const std::string& recognition_seq);
    
private:
    std::unordered_map<std::string, std::string> restriction_sites_;
    void initialize_common_sites();
};

// ============================================================================
// Sequence Complexity
// Measures linguistic complexity (low complexity = repetitive)
// ============================================================================

class SequenceComplexityMetric : public ScoreMetric {
public:
    explicit SequenceComplexityMetric(size_t window_size = 64);
    
    ScoreResult evaluate(const Sequence& seq) const override;
    std::string name() const override { return metrics::SEQUENCE_COMPLEXITY; }
    std::string description() const override { return "Sequence complexity (0-1, higher is more complex)"; }
    bool is_applicable(SequenceType type) const override;
    
private:
    size_t window_size_;
    double calculate_entropy(const std::string& window) const;
};

// ============================================================================
// Homopolymer Runs
// Detects long runs of single nucleotides (problematic for sequencing)
// ============================================================================

class HomopolymerRunsMetric : public ScoreMetric {
public:
    explicit HomopolymerRunsMetric(size_t threshold = 4);
    
    ScoreResult evaluate(const Sequence& seq) const override;
    std::string name() const override { return metrics::HOMOPOLYMER_RUNS; }
    std::string description() const override { return "Count of homopolymer runs above threshold"; }
    bool is_applicable(SequenceType type) const override;
    
private:
    size_t threshold_;
};

// ============================================================================
// Dinucleotide Odds Ratio
// CpG islands and other dinucleotide biases
// ============================================================================

class DinucleotideOddsMetric : public ScoreMetric {
public:
    explicit DinucleotideOddsMetric(const std::string& dinucleotide = "CG");
    
    ScoreResult evaluate(const Sequence& seq) const override;
    std::string name() const override { return metrics::DINUCLEOTIDE_ODDS; }
    std::string description() const override { return "Dinucleotide odds ratio"; }
    bool is_applicable(SequenceType type) const override;
    
private:
    std::string dinucleotide_;
};

// ============================================================================
// Rare Codons
// Counts codons that are rarely used (may slow translation)
// ============================================================================

class RareCodensMetric : public ScoreMetric {
public:
    RareCodensMetric();
    
    ScoreResult evaluate(const Sequence& seq) const override;
    std::string name() const override { return metrics::RARE_CODONS; }
    std::string description() const override { return "Percentage of rare codons"; }
    bool is_applicable(SequenceType type) const override;
    
private:
    std::set<std::string> rare_codons_;
    void initialize_rare_codons();
};

// ============================================================================
// Secondary Structure Prediction (simplified)
// Basic hairpin/stem-loop detection
// ============================================================================

class SecondaryStructureMetric : public ScoreMetric {
public:
    explicit SecondaryStructureMetric(size_t min_stem_length = 4);
    
    ScoreResult evaluate(const Sequence& seq) const override;
    std::string name() const override { return metrics::SECONDARY_STRUCTURE; }
    std::string description() const override { return "Secondary structure stability estimate"; }
    bool is_applicable(SequenceType type) const override;
    
private:
    size_t min_stem_length_;
    double estimate_folding_energy(const Sequence& seq) const;
};

} // namespace helixforge
