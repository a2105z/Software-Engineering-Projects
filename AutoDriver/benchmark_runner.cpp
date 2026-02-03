// benchmark_runner.cpp - Standalone benchmark executable

#include <iostream>
#include "benchmark/benchmark_suite.h"

int main(int argc, char** argv) {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║           AutoDriver - Performance Benchmark Suite         ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "This benchmark suite will test A*, RRT, and RRT* algorithms\n";
    std::cout << "across multiple grid sizes with various obstacle densities.\n\n";
    
    // Configure benchmarks
    BenchmarkConfig config;
    
    if (argc > 1 && std::string(argv[1]) == "--quick") {
        std::cout << "Running QUICK benchmarks...\n";
        config.grid_sizes = {10, 20};
        config.num_trials = 3;
    } else if (argc > 1 && std::string(argv[1]) == "--comprehensive") {
        std::cout << "Running COMPREHENSIVE benchmarks...\n";
        config.grid_sizes = {10, 20, 30, 50, 100};
        config.num_trials = 10;
    } else {
        std::cout << "Running STANDARD benchmarks...\n";
        config.grid_sizes = {10, 20, 30, 50};
        config.num_trials = 5;
    }
    
    config.obstacle_density = 0.2f;
    
    std::cout << "Configuration:\n";
    std::cout << "  Grid sizes: ";
    for (size_t i = 0; i < config.grid_sizes.size(); i++) {
        std::cout << config.grid_sizes[i] << "x" << config.grid_sizes[i];
        if (i < config.grid_sizes.size() - 1) std::cout << ", ";
    }
    std::cout << "\n";
    std::cout << "  Trials per size: " << config.num_trials << "\n";
    std::cout << "  Obstacle density: " << (config.obstacle_density * 100) << "%\n\n";
    
    std::cout << "Starting benchmarks...\n";
    std::cout << "═══════════════════════════════════════════════════════════\n\n";
    
    BenchmarkSuite suite(config);
    suite.runAll();
    
    std::cout << "\n═══════════════════════════════════════════════════════════\n";
    std::cout << "Generating reports...\n\n";
    
    suite.generateReport("benchmark_report.txt");
    suite.generateCSV("benchmark_results.csv");
    
    std::cout << "\n╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                 BENCHMARK COMPLETE!                        ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "Results saved:\n";
    std::cout << "  📄 benchmark_report.txt - Human-readable report\n";
    std::cout << "  📊 benchmark_results.csv - Data for Excel/analysis\n\n";
    
    std::cout << "Usage:\n";
    std::cout << "  benchmark              # Standard benchmarks\n";
    std::cout << "  benchmark --quick      # Quick test (2 sizes, 3 trials)\n";
    std::cout << "  benchmark --comprehensive  # Full test (5 sizes, 10 trials)\n\n";
    
    return 0;
}
