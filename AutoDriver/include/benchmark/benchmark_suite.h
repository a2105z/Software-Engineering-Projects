#pragma once

#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include "core/grid.h"
#include "core/astar.h"
#include "core/rrt.h"

/**
 * Benchmark result for a single test.
 */
struct BenchmarkResult {
    std::string test_name;
    std::string algorithm;
    int grid_size;
    double time_ms;
    int nodes_expanded;
    float path_cost;
    bool success;
    int iterations;
    
    BenchmarkResult(const std::string& name, const std::string& algo)
        : test_name(name), algorithm(algo), grid_size(0), time_ms(0.0),
          nodes_expanded(0), path_cost(0.0f), success(false), iterations(1) {}
};

/**
 * Benchmark configuration.
 */
struct BenchmarkConfig {
    std::vector<int> grid_sizes = {10, 20, 50, 100};
    int num_trials = 5;
    bool include_obstacles = true;
    float obstacle_density = 0.2f;
    
    BenchmarkConfig() = default;
};

/**
 * Automated benchmark suite for path planning algorithms.
 */
class BenchmarkSuite {
public:
    explicit BenchmarkSuite(const BenchmarkConfig& config = BenchmarkConfig());
    
    // Run all benchmarks
    void runAll();
    
    // Run specific algorithm benchmarks
    void benchmarkAStar();
    void benchmarkRRT();
    void benchmarkRRTStar();
    void benchmarkComparison();
    
    // Generate report
    void generateReport(const std::string& filename = "benchmark_report.txt");
    void generateCSV(const std::string& filename = "benchmark_results.csv");
    
    // Get results
    const std::vector<BenchmarkResult>& getResults() const { return results_; }
    
private:
    BenchmarkConfig config_;
    std::vector<BenchmarkResult> results_;
    
    // Helper functions
    Grid createTestGrid(int size, float obstacle_density);
    void addResult(const BenchmarkResult& result);
    double measureTime(std::function<void()> func);
    void printProgress(const std::string& message);
};

/**
 * Performance metrics calculator.
 */
class PerformanceMetrics {
public:
    static double calculateThroughput(int nodes, double time_ms);
    static double calculateSpeedup(double baseline_ms, double optimized_ms);
    static double calculateAverageTime(const std::vector<BenchmarkResult>& results);
    static void printSummary(const std::vector<BenchmarkResult>& results);
};
