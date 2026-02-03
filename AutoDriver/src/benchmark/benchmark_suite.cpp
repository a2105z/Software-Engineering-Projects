#include "benchmark/benchmark_suite.h"
#include "core/rrt.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <numeric>

BenchmarkSuite::BenchmarkSuite(const BenchmarkConfig& config)
    : config_(config) {}

Grid BenchmarkSuite::createTestGrid(int size, float obstacle_density) {
    Grid grid(size, size);
    
    if (config_.include_obstacles) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                if (dist(gen) < obstacle_density) {
                    grid.setObstacle(x, y, true);
                }
            }
        }
    }
    
    return grid;
}

double BenchmarkSuite::measureTime(std::function<void()> func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double, std::milli> duration = end - start;
    return duration.count();
}

void BenchmarkSuite::addResult(const BenchmarkResult& result) {
    results_.push_back(result);
}

void BenchmarkSuite::printProgress(const std::string& message) {
    std::cout << "[BENCHMARK] " << message << std::endl;
}

void BenchmarkSuite::benchmarkAStar() {
    printProgress("Starting A* benchmarks...");
    
    for (int size : config_.grid_sizes) {
        printProgress("Testing A* on " + std::to_string(size) + "x" + std::to_string(size) + " grid...");
        
        std::vector<double> times;
        std::vector<int> nodes;
        std::vector<float> costs;
        int successes = 0;
        
        for (int trial = 0; trial < config_.num_trials; trial++) {
            Grid grid = createTestGrid(size, config_.obstacle_density);
            AStar planner(grid);
            
            Vec2i start(size / 4, size / 4);
            Vec2i goal(size * 3 / 4, size * 3 / 4);
            
            AStarResult result;
            double time = measureTime([&]() {
                result = planner.findPath(start, goal);
            });
            
            if (result.success) {
                times.push_back(time);
                nodes.push_back(result.nodes_expanded);
                costs.push_back(result.path_cost);
                successes++;
            }
        }
        
        if (!times.empty()) {
            double avg_time = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
            int avg_nodes = std::accumulate(nodes.begin(), nodes.end(), 0) / nodes.size();
            float avg_cost = std::accumulate(costs.begin(), costs.end(), 0.0f) / costs.size();
            
            BenchmarkResult bench_result("A*_Grid_" + std::to_string(size), "A*");
            bench_result.grid_size = size;
            bench_result.time_ms = avg_time;
            bench_result.nodes_expanded = avg_nodes;
            bench_result.path_cost = avg_cost;
            bench_result.success = true;
            bench_result.iterations = config_.num_trials;
            
            addResult(bench_result);
            
            std::cout << "  Avg time: " << std::fixed << std::setprecision(2) 
                      << avg_time << "ms, Nodes: " << avg_nodes 
                      << ", Success rate: " << successes << "/" << config_.num_trials << std::endl;
        }
    }
}

void BenchmarkSuite::benchmarkRRT() {
    printProgress("Starting RRT benchmarks...");
    
    for (int size : config_.grid_sizes) {
        if (size > 50) continue; // RRT is slower on large grids
        
        printProgress("Testing RRT on " + std::to_string(size) + "x" + std::to_string(size) + " grid...");
        
        std::vector<double> times;
        std::vector<int> iters;
        std::vector<float> costs;
        int successes = 0;
        
        for (int trial = 0; trial < config_.num_trials; trial++) {
            Grid grid = createTestGrid(size, config_.obstacle_density * 0.5f); // Less obstacles for RRT
            RRT planner(grid);
            
            Vec2 start(static_cast<float>(size) / 4.0f, static_cast<float>(size) / 4.0f);
            Vec2 goal(static_cast<float>(size) * 3.0f / 4.0f, static_cast<float>(size) * 3.0f / 4.0f);
            
            RRTResult result;
            double time = measureTime([&]() {
                result = planner.findPath(start, goal, 2000);
            });
            
            if (result.success) {
                times.push_back(time);
                iters.push_back(result.iterations);
                costs.push_back(result.path_cost);
                successes++;
            }
        }
        
        if (!times.empty()) {
            double avg_time = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
            int avg_iters = std::accumulate(iters.begin(), iters.end(), 0) / iters.size();
            float avg_cost = std::accumulate(costs.begin(), costs.end(), 0.0f) / costs.size();
            
            BenchmarkResult bench_result("RRT_Grid_" + std::to_string(size), "RRT");
            bench_result.grid_size = size;
            bench_result.time_ms = avg_time;
            bench_result.nodes_expanded = avg_iters;
            bench_result.path_cost = avg_cost;
            bench_result.success = true;
            bench_result.iterations = config_.num_trials;
            
            addResult(bench_result);
            
            std::cout << "  Avg time: " << std::fixed << std::setprecision(2) 
                      << avg_time << "ms, Iterations: " << avg_iters 
                      << ", Success rate: " << successes << "/" << config_.num_trials << std::endl;
        }
    }
}

void BenchmarkSuite::benchmarkRRTStar() {
    printProgress("Starting RRT* benchmarks...");
    
    for (int size : config_.grid_sizes) {
        if (size > 50) continue; // RRT* is even slower
        
        printProgress("Testing RRT* on " + std::to_string(size) + "x" + std::to_string(size) + " grid...");
        
        std::vector<double> times;
        std::vector<int> iters;
        std::vector<float> costs;
        int successes = 0;
        
        for (int trial = 0; trial < config_.num_trials; trial++) {
            Grid grid = createTestGrid(size, config_.obstacle_density * 0.5f);
            RRTStar planner(grid);
            
            Vec2 start(static_cast<float>(size) / 4.0f, static_cast<float>(size) / 4.0f);
            Vec2 goal(static_cast<float>(size) * 3.0f / 4.0f, static_cast<float>(size) * 3.0f / 4.0f);
            
            RRTResult result;
            double time = measureTime([&]() {
                result = planner.findPath(start, goal, 2000);
            });
            
            if (result.success) {
                times.push_back(time);
                iters.push_back(result.iterations);
                costs.push_back(result.path_cost);
                successes++;
            }
        }
        
        if (!times.empty()) {
            double avg_time = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
            int avg_iters = std::accumulate(iters.begin(), iters.end(), 0) / iters.size();
            float avg_cost = std::accumulate(costs.begin(), costs.end(), 0.0f) / costs.size();
            
            BenchmarkResult bench_result("RRTStar_Grid_" + std::to_string(size), "RRT*");
            bench_result.grid_size = size;
            bench_result.time_ms = avg_time;
            bench_result.nodes_expanded = avg_iters;
            bench_result.path_cost = avg_cost;
            bench_result.success = true;
            bench_result.iterations = config_.num_trials;
            
            addResult(bench_result);
            
            std::cout << "  Avg time: " << std::fixed << std::setprecision(2) 
                      << avg_time << "ms, Iterations: " << avg_iters 
                      << ", Success rate: " << successes << "/" << config_.num_trials << std::endl;
        }
    }
}

void BenchmarkSuite::benchmarkComparison() {
    printProgress("Running comparison benchmark on 30x30 grid...");
    
    Grid grid = createTestGrid(30, 0.2f);
    
    // A*
    {
        AStar planner(grid);
        Vec2i start(5, 5);
        Vec2i goal(25, 25);
        
        AStarResult result;
        double time = measureTime([&]() {
            result = planner.findPath(start, goal);
        });
        
        BenchmarkResult bench_result("Comparison_AStar", "A*");
        bench_result.grid_size = 30;
        bench_result.time_ms = time;
        bench_result.nodes_expanded = result.nodes_expanded;
        bench_result.path_cost = result.path_cost;
        bench_result.success = result.success;
        addResult(bench_result);
    }
    
    // RRT
    {
        RRT planner(grid);
        Vec2 start(5.0f, 5.0f);
        Vec2 goal(25.0f, 25.0f);
        
        RRTResult result;
        double time = measureTime([&]() {
            result = planner.findPath(start, goal, 3000);
        });
        
        BenchmarkResult bench_result("Comparison_RRT", "RRT");
        bench_result.grid_size = 30;
        bench_result.time_ms = time;
        bench_result.nodes_expanded = result.iterations;
        bench_result.path_cost = result.path_cost;
        bench_result.success = result.success;
        addResult(bench_result);
    }
    
    // RRT*
    {
        RRTStar planner(grid);
        Vec2 start(5.0f, 5.0f);
        Vec2 goal(25.0f, 25.0f);
        
        RRTResult result;
        double time = measureTime([&]() {
            result = planner.findPath(start, goal, 3000);
        });
        
        BenchmarkResult bench_result("Comparison_RRTStar", "RRT*");
        bench_result.grid_size = 30;
        bench_result.time_ms = time;
        bench_result.nodes_expanded = result.iterations;
        bench_result.path_cost = result.path_cost;
        bench_result.success = result.success;
        addResult(bench_result);
    }
}

void BenchmarkSuite::runAll() {
    printProgress("=== Starting Automated Benchmark Suite ===\n");
    
    results_.clear();
    
    benchmarkAStar();
    std::cout << std::endl;
    
    benchmarkRRT();
    std::cout << std::endl;
    
    benchmarkRRTStar();
    std::cout << std::endl;
    
    benchmarkComparison();
    std::cout << std::endl;
    
    printProgress("=== Benchmark Suite Complete ===\n");
    
    PerformanceMetrics::printSummary(results_);
}

void BenchmarkSuite::generateReport(const std::string& filename) {
    std::ofstream file(filename);
    
    file << "=================================================\n";
    file << "AutoDriver - Benchmark Report\n";
    file << "=================================================\n\n";
    
    file << "Configuration:\n";
    file << "  Trials per test: " << config_.num_trials << "\n";
    file << "  Grid sizes: ";
    for (size_t i = 0; i < config_.grid_sizes.size(); i++) {
        file << config_.grid_sizes[i];
        if (i < config_.grid_sizes.size() - 1) file << ", ";
    }
    file << "\n";
    file << "  Obstacle density: " << config_.obstacle_density << "\n\n";
    
    file << "Results:\n";
    file << "-------------------------------------------------\n";
    file << std::left << std::setw(30) << "Test Name" 
         << std::setw(10) << "Algorithm" 
         << std::setw(10) << "Grid" 
         << std::setw(12) << "Time (ms)" 
         << std::setw(12) << "Nodes" 
         << std::setw(12) << "Cost" << "\n";
    file << "-------------------------------------------------\n";
    
    for (const auto& result : results_) {
        file << std::left << std::setw(30) << result.test_name
             << std::setw(10) << result.algorithm
             << std::setw(10) << result.grid_size
             << std::setw(12) << std::fixed << std::setprecision(2) << result.time_ms
             << std::setw(12) << result.nodes_expanded
             << std::setw(12) << std::fixed << std::setprecision(2) << result.path_cost << "\n";
    }
    
    file << "\n";
    file.close();
    
    std::cout << "Report saved to: " << filename << std::endl;
}

void BenchmarkSuite::generateCSV(const std::string& filename) {
    std::ofstream file(filename);
    
    file << "TestName,Algorithm,GridSize,TimeMs,NodesExpanded,PathCost,Success\n";
    
    for (const auto& result : results_) {
        file << result.test_name << ","
             << result.algorithm << ","
             << result.grid_size << ","
             << std::fixed << std::setprecision(4) << result.time_ms << ","
             << result.nodes_expanded << ","
             << std::fixed << std::setprecision(4) << result.path_cost << ","
             << (result.success ? "1" : "0") << "\n";
    }
    
    file.close();
    std::cout << "CSV saved to: " << filename << std::endl;
}

// PerformanceMetrics implementation

double PerformanceMetrics::calculateThroughput(int nodes, double time_ms) {
    if (time_ms == 0) return 0.0;
    return (nodes / time_ms) * 1000.0; // nodes per second
}

double PerformanceMetrics::calculateSpeedup(double baseline_ms, double optimized_ms) {
    if (optimized_ms == 0) return 0.0;
    return baseline_ms / optimized_ms;
}

double PerformanceMetrics::calculateAverageTime(const std::vector<BenchmarkResult>& results) {
    if (results.empty()) return 0.0;
    double sum = 0.0;
    for (const auto& result : results) {
        sum += result.time_ms;
    }
    return sum / results.size();
}

void PerformanceMetrics::printSummary(const std::vector<BenchmarkResult>& results) {
    std::cout << "\n=== Performance Summary ===\n\n";
    
    // Group by algorithm
    std::vector<std::string> algorithms = {"A*", "RRT", "RRT*"};
    
    for (const auto& algo : algorithms) {
        std::vector<BenchmarkResult> algo_results;
        for (const auto& result : results) {
            if (result.algorithm == algo) {
                algo_results.push_back(result);
            }
        }
        
        if (algo_results.empty()) continue;
        
        double avg_time = calculateAverageTime(algo_results);
        double total_nodes = 0;
        for (const auto& r : algo_results) {
            total_nodes += r.nodes_expanded;
        }
        
        double throughput = calculateThroughput(static_cast<int>(total_nodes), avg_time * algo_results.size());
        
        std::cout << algo << ":\n";
        std::cout << "  Average time: " << std::fixed << std::setprecision(2) << avg_time << " ms\n";
        std::cout << "  Throughput: " << std::fixed << std::setprecision(0) << throughput << " nodes/sec\n";
        std::cout << "  Tests run: " << algo_results.size() << "\n\n";
    }
}
