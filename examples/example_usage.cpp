/**
 * HelixForge Example Usage
 * 
 * Demonstrates core functionality of the HelixForge library
 */

#include "../src/phase1_core/sequence.hpp"
#include "../src/phase2_evaluation/scoring.hpp"
#include "../src/phase2_evaluation/constraint_engine.hpp"
#include "../src/phase3_mutation/mutation_engine.hpp"
#include "../src/phase4_optimization/optimizer.hpp"

#include <iostream>
#include <iomanip>

using namespace helixforge;

void example_1_basic_sequence_operations() {
    std::cout << "\n=== Example 1: Basic Sequence Operations ===\n";
    
    // Create a DNA sequence
    auto seq = Sequence::from_dna("ATGGCCCCCAAGTCCAGTCCACAGCC", "gene1", "Example gene");
    
    std::cout << "Original:            " << seq.data() << "\n";
    std::cout << "Length:              " << seq.length() << "\n";
    std::cout << "GC Content:          " << std::fixed << std::setprecision(2) 
              << seq.gc_content() * 100 << "%\n";
    std::cout << "Reverse Complement:  " << seq.reverse_complement().data() << "\n";
    std::cout << "RNA:                 " << seq.transcribe().data() << "\n";
    std::cout << "Protein:             " << seq.translate().data() << "\n";
}

void example_2_evaluation() {
    std::cout << "\n=== Example 2: Sequence Evaluation ===\n";
    
    auto seq = Sequence::from_dna("ATGGCCCCCAAGTCCAGTCCACAGCCGGCGAGGAGGGCTCG", "test_seq");
    
    // Create scoring engine
    ScoringEngine engine;
    engine.register_metric(metrics::GC_CONTENT);
    engine.register_metric(metrics::CODON_ADAPTATION_INDEX);
    engine.register_metric(metrics::SEQUENCE_COMPLEXITY);
    
    // Evaluate
    auto report = engine.evaluate(seq);
    std::cout << report.to_string();
}

void example_3_constraints() {
    std::cout << "\n=== Example 3: Constraint Checking ===\n";
    
    auto seq = Sequence::from_dna("ATGGAATTCAAGTCCAGTCCACAGCC", "constrained_seq");
    
    // Define constraints
    ConstraintEngine constraints;
    constraints.add_constraint_from_string("gc=0.40-0.60");
    constraints.add_constraint_from_string("avoid:GAATTC");  // EcoRI site
    
    // Check
    auto report = constraints.evaluate(seq);
    std::cout << report.to_string();
}

void example_4_mutation() {
    std::cout << "\n=== Example 4: Controlled Mutation ===\n";
    
    auto seq = Sequence::from_dna("ATGGCCCCCAAGTCCAGTCCACAGCC", "mutation_test");
    
    // Create mutation engine with seed for reproducibility
    MutationEngine engine(42);
    
    // Configure
    MutationConfig config;
    config.substitution_rate = 0.8;
    config.insertion_rate = 0.1;
    config.deletion_rate = 0.1;
    engine.set_config(config);
    
    // Apply mutations
    auto result = engine.apply_n_mutations(seq, 5);
    
    std::cout << "Original: " << seq.data() << "\n";
    std::cout << "Mutated:  " << result.mutated_sequence.data() << "\n";
    std::cout << "Mutations applied: " << result.mutations.size() << "\n";
}

void example_5_optimization() {
    std::cout << "\n=== Example 5: Sequence Optimization ===\n";
    
    auto seq = Sequence::from_dna(
        "ATGGCCCCCAAGTCCAGTCCACAGCCGGCGAGGAGGGCTCGGTCACCACC", 
        "optimize_test"
    );
    
    // Create optimizer (simulated annealing with seed)
    auto optimizer = OptimizerFactory::create(OptimizerType::SIMULATED_ANNEALING, 42);
    
    // Configure
    OptimizationConfig config;
    config.max_iterations = 1000;
    config.verbose = true;
    config.log_frequency = 100;
    optimizer->set_config(config);
    
    // Define objective: maximize expression
    auto objective = ObjectiveFactory::maximize_expression();
    
    // Define constraints
    ConstraintEngine constraints;
    constraints.add_constraint_from_string("gc=0.45-0.60");
    
    // Optimize!
    std::cout << "Starting optimization...\n";
    auto result = optimizer->optimize(seq, objective, &constraints);
    
    std::cout << "\n" << result.to_string();
    std::cout << "Optimized sequence: " << result.best_sequence.data() << "\n";
}

void example_6_complete_workflow() {
    std::cout << "\n=== Example 6: Complete Workflow ===\n";
    
    // 1. Load sequence
    std::string fasta = R"(
>test_gene Example gene for optimization
ATGGCCCCCAAGTCCAGTCCACAGCCGGCGAGGAGGGCTCGGTCACCACCGCCCGCAACGTT
    )";
    
    auto sequences = Sequence::from_fasta(fasta);
    if (sequences.empty()) {
        std::cerr << "No sequences found\n";
        return;
    }
    auto seq = sequences[0];
    
    // 2. Evaluate baseline
    std::cout << "Baseline evaluation:\n";
    ScoringEngine eval_engine;
    eval_engine.register_metric(metrics::GC_CONTENT);
    eval_engine.register_metric(metrics::CODON_ADAPTATION_INDEX);
    auto baseline_report = eval_engine.evaluate(seq);
    std::cout << baseline_report.to_string() << "\n";
    
    // 3. Optimize
    std::cout << "Optimizing for expression...\n";
    auto optimizer = OptimizerFactory::create(OptimizerType::GENETIC_ALGORITHM, 42);
    
    OptimizationConfig config;
    config.max_iterations = 500;
    config.patience = 50;
    optimizer->set_config(config);
    
    auto objective = ObjectiveFactory::maximize_expression();
    auto result = optimizer->optimize(seq, objective);
    
    // 4. Evaluate optimized
    std::cout << "\nOptimized evaluation:\n";
    auto optimized_report = eval_engine.evaluate(result.best_sequence);
    std::cout << optimized_report.to_string();
    
    // 5. Compare
    std::cout << "\nImprovement:\n";
    std::cout << "Score: " << result.best_score << " (from baseline)\n";
    std::cout << "Iterations: " << result.iterations << "\n";
}

int main() {
    std::cout << "HelixForge Example Usage\n";
    std::cout << "========================\n";
    
    try {
        example_1_basic_sequence_operations();
        example_2_evaluation();
        example_3_constraints();
        example_4_mutation();
        example_5_optimization();
        example_6_complete_workflow();
        
        std::cout << "\n✓ All examples completed successfully!\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
