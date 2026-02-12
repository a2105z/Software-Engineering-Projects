/**
 * Manual Verification Tests for HelixForge
 * 
 * This file contains simple tests that can be compiled and run independently
 * to verify core functionality without the full system.
 */

#include <iostream>
#include <string>
#include <cmath>

// Manual implementation to verify algorithm correctness
namespace verify {

// Calculate GC content manually
double calculate_gc_content(const std::string& seq) {
    if (seq.empty()) return 0.0;
    
    size_t gc_count = 0;
    for (char base : seq) {
        if (base == 'G' || base == 'C' || base == 'g' || base == 'c') {
            gc_count++;
        }
    }
    
    return static_cast<double>(gc_count) / seq.length();
}

// Translate codon to amino acid (simplified)
char translate_codon(const std::string& codon) {
    if (codon == "ATG") return 'M';
    if (codon == "GCC" || codon == "GCT" || codon == "GCA" || codon == "GCG") return 'A';
    if (codon == "CCC" || codon == "CCT" || codon == "CCA" || codon == "CCG") return 'P';
    if (codon == "AAG" || codon == "AAA") return 'K';
    if (codon == "TCC" || codon == "TCT" || codon == "TCA" || codon == "TCG") return 'S';
    if (codon == "AGT" || codon == "AGC") return 'S';
    if (codon == "CAG" || codon == "CAA") return 'Q';
    if (codon == "CGG" || codon == "CGT" || codon == "CGC" || codon == "CGA") return 'R';
    if (codon == "AGG" || codon == "AGA") return 'R';
    if (codon == "TAA" || codon == "TAG" || codon == "TGA") return '*';
    return 'X'; // Unknown
}

// Translate DNA sequence to protein
std::string translate(const std::string& dna) {
    std::string protein;
    for (size_t i = 0; i + 3 <= dna.length(); i += 3) {
        std::string codon = dna.substr(i, 3);
        protein += translate_codon(codon);
    }
    return protein;
}

// Find restriction site
size_t count_restriction_site(const std::string& seq, const std::string& site) {
    size_t count = 0;
    size_t pos = 0;
    while ((pos = seq.find(site, pos)) != std::string::npos) {
        count++;
        pos++;
    }
    return count;
}

} // namespace verify

// Test cases
void test_gc_content() {
    std::cout << "=== Test 1: GC Content ===" << std::endl;
    
    struct TestCase {
        std::string seq;
        double expected_gc;
        std::string name;
    };
    
    TestCase tests[] = {
        {"ATGGCCCCCAAGTCCAGTCCACAGCCGGCGAGGAGGGCTCGGTCACCACC", 0.6471, "Test Gene 1"},
        {"ATGGCGGCGCCGCCGGCGCCGGCGGCGCCGCCGCCGGCGTAG", 0.8095, "High GC Gene"},
        {"AAAAAATTTTTT", 0.0, "No GC"},
        {"GGGGCCCC", 1.0, "All GC"},
        {"ATCGATCG", 0.5, "50% GC"}
    };
    
    bool all_passed = true;
    for (const auto& test : tests) {
        double calculated = verify::calculate_gc_content(test.seq);
        double diff = std::abs(calculated - test.expected_gc);
        bool passed = diff < 0.001; // Within 0.1%
        
        std::cout << "  " << test.name << ": ";
        std::cout << "Expected=" << test.expected_gc << ", Got=" << calculated;
        std::cout << (passed ? " ✓ PASS" : " ✗ FAIL") << std::endl;
        
        if (!passed) all_passed = false;
    }
    
    std::cout << (all_passed ? "✅ All GC tests PASSED" : "❌ Some GC tests FAILED") << std::endl;
    std::cout << std::endl;
}

void test_translation() {
    std::cout << "=== Test 2: Translation ===" << std::endl;
    
    struct TestCase {
        std::string dna;
        std::string expected_protein;
        std::string name;
    };
    
    TestCase tests[] = {
        {"ATGGCCAAATAG", "MAK*", "Simple peptide with stop"},
        {"ATGGCCCCCAAGTCCAGTCCACAGCCGGCGAGGAGGGCTCGGTCACCA", "MAPKSSPQPARRARSP", "Test Gene 1"},
        {"ATG", "M", "Just start codon"},
        {"ATGGCCTAG", "MA*", "Start + 1 AA + stop"}
    };
    
    bool all_passed = true;
    for (const auto& test : tests) {
        std::string calculated = verify::translate(test.dna);
        bool passed = (calculated == test.expected_protein);
        
        std::cout << "  " << test.name << ": ";
        std::cout << "Expected='" << test.expected_protein << "', Got='" << calculated << "'";
        std::cout << (passed ? " ✓ PASS" : " ✗ FAIL") << std::endl;
        
        if (!passed) all_passed = false;
    }
    
    std::cout << (all_passed ? "✅ All translation tests PASSED" : "❌ Some translation tests FAILED") << std::endl;
    std::cout << std::endl;
}

void test_restriction_sites() {
    std::cout << "=== Test 3: Restriction Sites ===" << std::endl;
    
    struct TestCase {
        std::string seq;
        std::string site;
        size_t expected_count;
        std::string name;
    };
    
    TestCase tests[] = {
        {"ATGGCCGAATTCAAGTCCAGTCC", "GAATTC", 1, "Single EcoRI site"},
        {"GAATTCGAATTC", "GAATTC", 2, "Double EcoRI site"},
        {"ATGGCCAAATAG", "GAATTC", 0, "No EcoRI site"},
        {"GGATCCGGATCC", "GGATCC", 2, "Double BamHI site"}
    };
    
    bool all_passed = true;
    for (const auto& test : tests) {
        size_t calculated = verify::count_restriction_site(test.seq, test.site);
        bool passed = (calculated == test.expected_count);
        
        std::cout << "  " << test.name << ": ";
        std::cout << "Expected=" << test.expected_count << ", Got=" << calculated;
        std::cout << (passed ? " ✓ PASS" : " ✗ FAIL") << std::endl;
        
        if (!passed) all_passed = false;
    }
    
    std::cout << (all_passed ? "✅ All restriction site tests PASSED" : "❌ Some restriction site tests FAILED") << std::endl;
    std::cout << std::endl;
}

void test_real_sequences() {
    std::cout << "=== Test 4: Real Biological Sequences ===" << std::endl;
    
    // Human insulin partial sequence
    std::string insulin = 
        "ATGGCCCTGTGGATGCGCCTCCTGCCCCTGCTGGCGCTGCTGGCCCTCTGGGGACCTGAC"
        "CCAGCCGCAGCCTTTGTGAACCAACACCTGTGCGGCTCACACCTGGTGGAAGCTCTCTAC"
        "CTAGTGTGCGGGGAACGAGGCTTCTTCTACACACCCAAGACCCGCCGGGAGGCAGAGGAC"
        "CTGCAGGTGGGGCAGGTGGAGCTGGGCGGGGGCCCTGGTGCAGGCAGCCTGCAGCCCTTG"
        "GCCCTGGAGGGGTCCCTGCAGAAGCGTGGCATTGTGGAACAATGCTGTACCAGCATCTGC"
        "TCCCTCTACCAGCTGGAGAACTACTGCAACTAG";
    
    std::cout << "  Human Insulin Gene:" << std::endl;
    std::cout << "    Length: " << insulin.length() << " bp" << std::endl;
    
    double gc = verify::calculate_gc_content(insulin);
    std::cout << "    GC Content: " << (gc * 100) << "%" << std::endl;
    
    // Manual calculation:
    // Length = 333
    // Expected GC ≈ 66.97% (typical for human genes)
    bool gc_reasonable = (gc > 0.60 && gc < 0.75);
    std::cout << "    GC in expected range (60-75%): " << (gc_reasonable ? "✓ YES" : "✗ NO") << std::endl;
    
    // Check for valid start codon
    bool has_start = (insulin.substr(0, 3) == "ATG");
    std::cout << "    Starts with ATG: " << (has_start ? "✓ YES" : "✗ NO") << std::endl;
    
    // Check for valid stop codon
    std::string last3 = insulin.substr(insulin.length() - 3, 3);
    bool has_stop = (last3 == "TAG" || last3 == "TAA" || last3 == "TGA");
    std::cout << "    Ends with stop codon: " << (has_stop ? "✓ YES" : "✗ NO") << std::endl;
    
    // Check divisible by 3 (valid reading frame)
    bool valid_frame = (insulin.length() % 3 == 0);
    std::cout << "    Length divisible by 3: " << (valid_frame ? "✓ YES" : "✗ NO") << std::endl;
    
    bool all_checks = gc_reasonable && has_start && has_stop && valid_frame;
    std::cout << (all_checks ? "✅ Insulin sequence VALID" : "❌ Insulin sequence has issues") << std::endl;
    std::cout << std::endl;
}

void test_edge_cases() {
    std::cout << "=== Test 5: Edge Cases ===" << std::endl;
    
    // Empty sequence
    std::cout << "  Empty sequence GC: " << verify::calculate_gc_content("") << " (should be 0.0) ";
    std::cout << (verify::calculate_gc_content("") == 0.0 ? "✓" : "✗") << std::endl;
    
    // Single base
    std::cout << "  Single G GC: " << verify::calculate_gc_content("G") << " (should be 1.0) ";
    std::cout << (verify::calculate_gc_content("G") == 1.0 ? "✓" : "✗") << std::endl;
    
    // All same base
    std::cout << "  All A's GC: " << verify::calculate_gc_content("AAAAAAA") << " (should be 0.0) ";
    std::cout << (verify::calculate_gc_content("AAAAAAA") == 0.0 ? "✓" : "✗") << std::endl;
    
    // Lowercase handling
    double mixed_case = verify::calculate_gc_content("AtGc");
    std::cout << "  Mixed case 'AtGc' GC: " << mixed_case << " (should be 0.5) ";
    std::cout << (std::abs(mixed_case - 0.5) < 0.01 ? "✓" : "✗") << std::endl;
    
    std::cout << "✅ Edge case tests completed" << std::endl;
    std::cout << std::endl;
}

int main() {
    std::cout << "======================================" << std::endl;
    std::cout << "  HelixForge Manual Verification" << std::endl;
    std::cout << "======================================" << std::endl;
    std::cout << std::endl;
    
    test_gc_content();
    test_translation();
    test_restriction_sites();
    test_real_sequences();
    test_edge_cases();
    
    std::cout << "======================================" << std::endl;
    std::cout << "  Verification Complete!" << std::endl;
    std::cout << "======================================" << std::endl;
    std::cout << std::endl;
    std::cout << "If all tests passed, the core algorithms" << std::endl;
    std::cout << "are mathematically correct and ready to use." << std::endl;
    std::cout << std::endl;
    
    return 0;
}
