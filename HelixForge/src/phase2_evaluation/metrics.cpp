#include "metrics.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace helixforge {

// ============================================================================
// GCContentMetric Implementation
// ============================================================================

ScoreResult GCContentMetric::evaluate(const Sequence& seq) const {
    double gc = seq.gc_content();
    return ScoreResult(name(), gc, "GC content ratio");
}

bool GCContentMetric::is_applicable(SequenceType type) const {
    return type == SequenceType::DNA || type == SequenceType::RNA;
}

// ============================================================================
// CodonAdaptationIndexMetric Implementation
// ============================================================================

CodonAdaptationIndexMetric::CodonAdaptationIndexMetric() {
    initialize_default_weights();
}

CodonAdaptationIndexMetric::CodonAdaptationIndexMetric(
    const std::unordered_map<std::string, double>& custom_weights)
    : codon_weights_(custom_weights) {}

void CodonAdaptationIndexMetric::initialize_default_weights() {
    // E. coli codon weights (simplified, based on highly expressed genes)
    // Higher weight = more frequently used in highly expressed genes
    
    // Phenylalanine
    codon_weights_["TTT"] = 0.42; codon_weights_["TTC"] = 0.58;
    // Leucine
    codon_weights_["TTA"] = 0.12; codon_weights_["TTG"] = 0.13;
    codon_weights_["CTT"] = 0.11; codon_weights_["CTC"] = 0.10;
    codon_weights_["CTA"] = 0.04; codon_weights_["CTG"] = 0.50;
    // Isoleucine
    codon_weights_["ATT"] = 0.49; codon_weights_["ATC"] = 0.42;
    codon_weights_["ATA"] = 0.09;
    // Methionine
    codon_weights_["ATG"] = 1.00;
    // Valine
    codon_weights_["GTT"] = 0.39; codon_weights_["GTC"] = 0.23;
    codon_weights_["GTA"] = 0.16; codon_weights_["GTG"] = 0.22;
    // Serine
    codon_weights_["TCT"] = 0.17; codon_weights_["TCC"] = 0.15;
    codon_weights_["TCA"] = 0.13; codon_weights_["TCG"] = 0.15;
    codon_weights_["AGT"] = 0.14; codon_weights_["AGC"] = 0.26;
    // Proline
    codon_weights_["CCT"] = 0.18; codon_weights_["CCC"] = 0.12;
    codon_weights_["CCA"] = 0.19; codon_weights_["CCG"] = 0.51;
    // Threonine
    codon_weights_["ACT"] = 0.19; codon_weights_["ACC"] = 0.44;
    codon_weights_["ACA"] = 0.14; codon_weights_["ACG"] = 0.23;
    // Alanine
    codon_weights_["GCT"] = 0.18; codon_weights_["GCC"] = 0.27;
    codon_weights_["GCA"] = 0.21; codon_weights_["GCG"] = 0.34;
    // Tyrosine
    codon_weights_["TAT"] = 0.43; codon_weights_["TAC"] = 0.57;
    // Stop
    codon_weights_["TAA"] = 0.64; codon_weights_["TAG"] = 0.09;
    codon_weights_["TGA"] = 0.27;
    // Histidine
    codon_weights_["CAT"] = 0.42; codon_weights_["CAC"] = 0.58;
    // Glutamine
    codon_weights_["CAA"] = 0.35; codon_weights_["CAG"] = 0.65;
    // Asparagine
    codon_weights_["AAT"] = 0.44; codon_weights_["AAC"] = 0.56;
    // Lysine
    codon_weights_["AAA"] = 0.76; codon_weights_["AAG"] = 0.24;
    // Aspartic acid
    codon_weights_["GAT"] = 0.62; codon_weights_["GAC"] = 0.38;
    // Glutamic acid
    codon_weights_["GAA"] = 0.69; codon_weights_["GAG"] = 0.31;
    // Cysteine
    codon_weights_["TGT"] = 0.44; codon_weights_["TGC"] = 0.56;
    // Tryptophan
    codon_weights_["TGG"] = 1.00;
    // Arginine
    codon_weights_["CGT"] = 0.38; codon_weights_["CGC"] = 0.40;
    codon_weights_["CGA"] = 0.06; codon_weights_["CGG"] = 0.10;
    codon_weights_["AGA"] = 0.03; codon_weights_["AGG"] = 0.03;
    // Glycine
    codon_weights_["GGT"] = 0.35; codon_weights_["GGC"] = 0.37;
    codon_weights_["GGA"] = 0.13; codon_weights_["GGG"] = 0.15;
}

ScoreResult CodonAdaptationIndexMetric::evaluate(const Sequence& seq) const {
    if (seq.length() < 3) {
        return ScoreResult(name(), 0.0, "Sequence too short for codon analysis");
    }
    
    std::string data = seq.data();
    // Convert RNA to DNA
    std::replace(data.begin(), data.end(), 'U', 'T');
    
    double log_sum = 0.0;
    size_t codon_count = 0;
    
    for (size_t i = 0; i + 3 <= data.length(); i += 3) {
        std::string codon = data.substr(i, 3);
        auto it = codon_weights_.find(codon);
        if (it != codon_weights_.end() && it->second > 0.0) {
            log_sum += std::log(it->second);
            ++codon_count;
        }
    }
    
    if (codon_count == 0) {
        return ScoreResult(name(), 0.0, "No valid codons found");
    }
    
    // CAI is the geometric mean of codon weights
    double cai = std::exp(log_sum / codon_count);
    
    ScoreResult result(name(), cai, "CAI based on E. coli codon usage");
    result.metadata["codon_count"] = static_cast<double>(codon_count);
    
    return result;
}

bool CodonAdaptationIndexMetric::is_applicable(SequenceType type) const {
    return type == SequenceType::DNA || type == SequenceType::RNA;
}

// ============================================================================
// CodonUsageBiasMetric Implementation
// ============================================================================

ScoreResult CodonUsageBiasMetric::evaluate(const Sequence& seq) const {
    if (seq.length() < 3) {
        return ScoreResult(name(), 0.0, "Sequence too short");
    }
    
    std::string data = seq.data();
    std::replace(data.begin(), data.end(), 'U', 'T');
    
    // Count codon frequencies
    std::unordered_map<std::string, size_t> codon_counts;
    size_t total_codons = 0;
    
    for (size_t i = 0; i + 3 <= data.length(); i += 3) {
        std::string codon = data.substr(i, 3);
        codon_counts[codon]++;
        ++total_codons;
    }
    
    if (total_codons == 0) {
        return ScoreResult(name(), 0.0);
    }
    
    // Calculate entropy (measure of bias)
    double entropy = 0.0;
    for (const auto& [codon, count] : codon_counts) {
        double p = static_cast<double>(count) / total_codons;
        entropy -= p * std::log(p);
    }
    
    // Normalize by maximum possible entropy (log of 61 sense codons)
    double max_entropy = std::log(61.0);
    double normalized_entropy = entropy / max_entropy;
    
    // Bias is inverse of normalized entropy
    double bias = 1.0 - normalized_entropy;
    
    ScoreResult result(name(), bias, "Codon usage bias (0=uniform, 1=maximum bias)");
    result.metadata["entropy"] = entropy;
    result.metadata["unique_codons"] = static_cast<double>(codon_counts.size());
    
    return result;
}

bool CodonUsageBiasMetric::is_applicable(SequenceType type) const {
    return type == SequenceType::DNA || type == SequenceType::RNA;
}

// ============================================================================
// ORFIntegrityMetric Implementation
// ============================================================================

ORFIntegrityMetric::ORFIntegrityMetric(size_t min_orf_length)
    : min_orf_length_(min_orf_length) {}

ScoreResult ORFIntegrityMetric::evaluate(const Sequence& seq) const {
    auto orfs = seq.find_orfs(min_orf_length_);
    
    double score = orfs.empty() ? 0.0 : 1.0;
    
    ScoreResult result(name(), score, "ORF integrity (0=no valid ORF, 1=valid ORF found)");
    result.metadata["orf_count"] = static_cast<double>(orfs.size());
    result.metadata["min_orf_length"] = static_cast<double>(min_orf_length_);
    
    return result;
}

bool ORFIntegrityMetric::is_applicable(SequenceType type) const {
    return type == SequenceType::DNA || type == SequenceType::RNA;
}

// ============================================================================
// RestrictionSitesMetric Implementation
// ============================================================================

RestrictionSitesMetric::RestrictionSitesMetric() {
    initialize_common_sites();
}

RestrictionSitesMetric::RestrictionSitesMetric(const std::vector<std::string>& custom_sites) {
    for (const auto& site : custom_sites) {
        restriction_sites_["Custom_" + site] = site;
    }
}

void RestrictionSitesMetric::initialize_common_sites() {
    // Common restriction enzyme recognition sites
    restriction_sites_["EcoRI"] = "GAATTC";
    restriction_sites_["BamHI"] = "GGATCC";
    restriction_sites_["HindIII"] = "AAGCTT";
    restriction_sites_["PstI"] = "CTGCAG";
    restriction_sites_["SalI"] = "GTCGAC";
    restriction_sites_["XbaI"] = "TCTAGA";
    restriction_sites_["NotI"] = "GCGGCCGC";
    restriction_sites_["XhoI"] = "CTCGAG";
    restriction_sites_["SacI"] = "GAGCTC";
    restriction_sites_["KpnI"] = "GGTACC";
    restriction_sites_["NcoI"] = "CCATGG";
    restriction_sites_["NdeI"] = "CATATG";
}

void RestrictionSitesMetric::add_site(const std::string& enzyme_name, const std::string& recognition_seq) {
    restriction_sites_[enzyme_name] = recognition_seq;
}

ScoreResult RestrictionSitesMetric::evaluate(const Sequence& seq) const {
    size_t total_sites = 0;
    ScoreResult result(name(), 0.0, "Number of restriction sites");
    
    for (const auto& [enzyme, site] : restriction_sites_) {
        size_t count = seq.count(site);
        total_sites += count;
        if (count > 0) {
            result.metadata[enzyme] = static_cast<double>(count);
        }
    }
    
    result.value = static_cast<double>(total_sites);
    
    return result;
}

bool RestrictionSitesMetric::is_applicable(SequenceType type) const {
    return type == SequenceType::DNA || type == SequenceType::RNA;
}

// ============================================================================
// SequenceComplexityMetric Implementation
// ============================================================================

SequenceComplexityMetric::SequenceComplexityMetric(size_t window_size)
    : window_size_(window_size) {}

double SequenceComplexityMetric::calculate_entropy(const std::string& window) const {
    std::unordered_map<char, size_t> freq;
    for (char c : window) {
        freq[c]++;
    }
    
    double entropy = 0.0;
    double n = static_cast<double>(window.length());
    
    for (const auto& [c, count] : freq) {
        double p = count / n;
        entropy -= p * std::log2(p);
    }
    
    return entropy;
}

ScoreResult SequenceComplexityMetric::evaluate(const Sequence& seq) const {
    if (seq.length() < window_size_) {
        double entropy = calculate_entropy(seq.data());
        double max_entropy = std::log2(4.0); // For DNA/RNA
        return ScoreResult(name(), entropy / max_entropy, "Sequence complexity");
    }
    
    double avg_entropy = 0.0;
    size_t window_count = 0;
    
    for (size_t i = 0; i + window_size_ <= seq.length(); i += window_size_ / 2) {
        std::string window = seq.data().substr(i, window_size_);
        avg_entropy += calculate_entropy(window);
        ++window_count;
    }
    
    if (window_count > 0) {
        avg_entropy /= window_count;
    }
    
    // Normalize by theoretical maximum (log2(4) for nucleotides)
    double max_entropy = std::log2(4.0);
    double complexity = avg_entropy / max_entropy;
    
    ScoreResult result(name(), complexity, "Average sequence complexity");
    result.metadata["window_size"] = static_cast<double>(window_size_);
    result.metadata["windows_analyzed"] = static_cast<double>(window_count);
    
    return result;
}

bool SequenceComplexityMetric::is_applicable(SequenceType type) const {
    return true; // Applicable to all sequence types
}

// ============================================================================
// HomopolymerRunsMetric Implementation
// ============================================================================

HomopolymerRunsMetric::HomopolymerRunsMetric(size_t threshold)
    : threshold_(threshold) {}

ScoreResult HomopolymerRunsMetric::evaluate(const Sequence& seq) const {
    if (seq.empty()) {
        return ScoreResult(name(), 0.0);
    }
    
    size_t run_count = 0;
    size_t max_run = 0;
    size_t current_run = 1;
    char current_char = seq[0];
    
    for (size_t i = 1; i < seq.length(); ++i) {
        if (seq[i] == current_char) {
            ++current_run;
        } else {
            if (current_run >= threshold_) {
                ++run_count;
            }
            max_run = std::max(max_run, current_run);
            current_run = 1;
            current_char = seq[i];
        }
    }
    
    // Check last run
    if (current_run >= threshold_) {
        ++run_count;
    }
    max_run = std::max(max_run, current_run);
    
    ScoreResult result(name(), static_cast<double>(run_count), 
                      "Homopolymer runs >= threshold");
    result.metadata["threshold"] = static_cast<double>(threshold_);
    result.metadata["max_run_length"] = static_cast<double>(max_run);
    
    return result;
}

bool HomopolymerRunsMetric::is_applicable(SequenceType type) const {
    return true;
}

// ============================================================================
// DinucleotideOddsMetric Implementation
// ============================================================================

DinucleotideOddsMetric::DinucleotideOddsMetric(const std::string& dinucleotide)
    : dinucleotide_(Alphabet::to_upper(dinucleotide)) {}

ScoreResult DinucleotideOddsMetric::evaluate(const Sequence& seq) const {
    if (seq.length() < 2 || dinucleotide_.length() != 2) {
        return ScoreResult(name(), 0.0);
    }
    
    // Count dinucleotide and individual nucleotides
    size_t dinuc_count = seq.count(dinucleotide_);
    size_t first_count = seq.count(dinucleotide_[0]);
    size_t second_count = seq.count(dinucleotide_[1]);
    
    size_t n = seq.length();
    
    if (first_count == 0 || second_count == 0 || n < 2) {
        return ScoreResult(name(), 0.0);
    }
    
    // Calculate expected frequency
    double p_first = static_cast<double>(first_count) / n;
    double p_second = static_cast<double>(second_count) / n;
    double expected = p_first * p_second * (n - 1);
    
    // Calculate odds ratio
    double odds = (expected > 0.0) ? dinuc_count / expected : 0.0;
    
    ScoreResult result(name(), odds, "Dinucleotide odds ratio (>1 = enriched)");
    result.metadata["dinucleotide"] = static_cast<double>(dinuc_count);
    result.metadata["expected"] = expected;
    
    return result;
}

bool DinucleotideOddsMetric::is_applicable(SequenceType type) const {
    return type == SequenceType::DNA || type == SequenceType::RNA;
}

// ============================================================================
// RareCodensMetric Implementation
// ============================================================================

RareCodensMetric::RareCodensMetric() {
    initialize_rare_codons();
}

void RareCodensMetric::initialize_rare_codons() {
    // E. coli rare codons (frequency < 5%)
    rare_codons_ = {
        "AGA", "AGG", "ATA", "CTA", "CGA", "CGG", "TCG"
    };
}

ScoreResult RareCodensMetric::evaluate(const Sequence& seq) const {
    if (seq.length() < 3) {
        return ScoreResult(name(), 0.0);
    }
    
    std::string data = seq.data();
    std::replace(data.begin(), data.end(), 'U', 'T');
    
    size_t rare_count = 0;
    size_t total_codons = 0;
    
    for (size_t i = 0; i + 3 <= data.length(); i += 3) {
        std::string codon = data.substr(i, 3);
        ++total_codons;
        if (rare_codons_.count(codon) > 0) {
            ++rare_count;
        }
    }
    
    double percentage = (total_codons > 0) ? 
        (static_cast<double>(rare_count) / total_codons) * 100.0 : 0.0;
    
    ScoreResult result(name(), percentage, "Percentage of rare codons");
    result.metadata["rare_count"] = static_cast<double>(rare_count);
    result.metadata["total_codons"] = static_cast<double>(total_codons);
    
    return result;
}

bool RareCodensMetric::is_applicable(SequenceType type) const {
    return type == SequenceType::DNA || type == SequenceType::RNA;
}

// ============================================================================
// SecondaryStructureMetric Implementation
// ============================================================================

SecondaryStructureMetric::SecondaryStructureMetric(size_t min_stem_length)
    : min_stem_length_(min_stem_length) {}

double SecondaryStructureMetric::estimate_folding_energy(const Sequence& seq) const {
    // Simplified folding energy estimate
    // Count potential base pairs (simplified hairpin detection)
    
    double energy = 0.0;
    const std::string& data = seq.data();
    
    // Simple nearest-neighbor approximation
    for (size_t i = 0; i < data.length() - 1; ++i) {
        char pair[2] = {data[i], data[i + 1]};
        
        // GC pairs contribute more stability (more negative energy)
        if ((pair[0] == 'G' && pair[1] == 'C') || (pair[0] == 'C' && pair[1] == 'G')) {
            energy -= 3.0;
        } else if ((pair[0] == 'A' && pair[1] == 'T') || (pair[0] == 'T' && pair[1] == 'A') ||
                   (pair[0] == 'A' && pair[1] == 'U') || (pair[0] == 'U' && pair[1] == 'A')) {
            energy -= 2.0;
        }
    }
    
    return energy;
}

ScoreResult SecondaryStructureMetric::evaluate(const Sequence& seq) const {
    double energy = estimate_folding_energy(seq);
    
    // Normalize by sequence length
    double normalized_energy = seq.length() > 0 ? energy / seq.length() : 0.0;
    
    ScoreResult result(name(), normalized_energy, 
                      "Estimated folding energy (more negative = more stable)");
    result.metadata["raw_energy"] = energy;
    result.metadata["length"] = static_cast<double>(seq.length());
    
    return result;
}

bool SecondaryStructureMetric::is_applicable(SequenceType type) const {
    return type == SequenceType::DNA || type == SequenceType::RNA;
}

} // namespace helixforge
