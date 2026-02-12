#pragma once

#include "../phase1_core/sequence.hpp"
#include "../phase4_optimization/optimizer.hpp"
#include <string>
#include <memory>
#include <vector>
#include <ctime>

// Forward declare SQLite types to avoid including sqlite3.h in header
struct sqlite3;

namespace helixforge {

/**
 * Experiment metadata
 */
struct ExperimentMetadata {
    std::string experiment_id;
    std::string name;
    std::string description;
    std::string algorithm;
    std::string objective;
    std::time_t timestamp;
    uint64_t seed;
    
    // Configuration as JSON string
    std::string config_json;
    
    ExperimentMetadata()
        : timestamp(std::time(nullptr)), seed(0) {}
};

/**
 * SQLite-based experiment logger
 * Logs optimization runs for reproducibility and analysis
 */
class ExperimentLog {
public:
    explicit ExperimentLog(const std::string& db_path);
    ~ExperimentLog();
    
    // Disable copy
    ExperimentLog(const ExperimentLog&) = delete;
    ExperimentLog& operator=(const ExperimentLog&) = delete;
    
    // Open/close database
    bool open();
    void close();
    bool is_open() const { return db_ != nullptr; }
    
    // Create tables if they don't exist
    bool initialize_schema();
    
    // Log experiment
    std::string create_experiment(const ExperimentMetadata& metadata);
    
    // Log optimization results
    bool log_optimization(
        const std::string& experiment_id,
        const Sequence& initial_sequence,
        const OptimizationResult& result
    );
    
    // Log iteration data
    bool log_iteration(
        const std::string& experiment_id,
        size_t iteration,
        double score,
        const std::string& sequence_data
    );
    
    // Query experiments
    std::vector<ExperimentMetadata> list_experiments(size_t limit = 100) const;
    ExperimentMetadata get_experiment(const std::string& experiment_id) const;
    
    // Query results
    OptimizationResult get_optimization_result(const std::string& experiment_id) const;
    std::vector<double> get_score_history(const std::string& experiment_id) const;
    
    // Export experiment to JSON
    std::string export_experiment_json(const std::string& experiment_id) const;
    
    // Delete experiment
    bool delete_experiment(const std::string& experiment_id);
    
    // Database maintenance
    bool vacuum();
    size_t get_database_size() const;
    
private:
    std::string db_path_;
    sqlite3* db_;
    
    // Helper methods
    bool execute(const std::string& sql);
    std::string generate_experiment_id() const;
    
    // Schema
    static const char* SCHEMA_SQL;
};

/**
 * Experiment comparison utilities
 */
class ExperimentAnalysis {
public:
    // Compare multiple experiments
    static std::string compare_experiments(
        const std::vector<std::string>& experiment_ids,
        ExperimentLog& log
    );
    
    // Generate summary statistics
    static std::string summarize_experiment(
        const std::string& experiment_id,
        ExperimentLog& log
    );
    
    // Export to CSV for external analysis
    static bool export_to_csv(
        const std::string& experiment_id,
        ExperimentLog& log,
        const std::string& output_path
    );
};

} // namespace helixforge
