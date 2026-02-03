#pragma once

#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include "astar.h"
#include "grid.h"

/**
 * Object pool for node allocation (avoids frequent new/delete).
 */
template<typename T>
class ObjectPool {
public:
    explicit ObjectPool(size_t initial_size = 100) {
        objects_.reserve(initial_size);
        // Don't pre-allocate - will create on demand
    }
    
    T* acquire() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (available_.empty()) {
            objects_.push_back(std::make_unique<T>());
            return objects_.back().get();
        }
        T* obj = available_.back();
        available_.pop_back();
        return obj;
    }
    
    void release(T* obj) {
        std::lock_guard<std::mutex> lock(mutex_);
        available_.push_back(obj);
    }
    
    size_t size() const { return objects_.size(); }
    
private:
    std::vector<std::unique_ptr<T>> objects_;
    std::vector<T*> available_;
    std::mutex mutex_;
};

/**
 * Parallel A* search using bidirectional search.
 */
class ParallelAStar {
public:
    explicit ParallelAStar(const Grid& grid);
    
    // Find path using bidirectional parallel search
    AStarResult findPath(Vec2i start, Vec2i goal);
    
    void setNumThreads(int n) { num_threads_ = n; }
    
private:
    const Grid& grid_;
    int num_threads_;
    
    // Forward and backward search from start and goal
    AStarResult searchForward(Vec2i start, Vec2i goal, int max_iterations);
    AStarResult searchBackward(Vec2i start, Vec2i goal, int max_iterations);
};

/**
 * Anytime Repairing A* (ARA*) - finds solution quickly, refines over time.
 */
class ARAStarPlanner {
public:
    explicit ARAStarPlanner(const Grid& grid);
    
    // Find initial path quickly, then refine
    AStarResult findPath(Vec2i start, Vec2i goal, float initial_epsilon = 5.0f);
    
    // Continue refining existing solution
    bool improvePath(float epsilon_decrease = 0.5f);
    
    // Get current best path
    const std::vector<Vec2i>& getCurrentPath() const { return current_path_; }
    
private:
    const Grid& grid_;
    std::unique_ptr<AStar> planner_;
    std::vector<Vec2i> current_path_;
    float current_epsilon_;
    Vec2i start_;
    Vec2i goal_;
    
    // Weighted A* with inflation factor
    AStarResult weightedAStar(Vec2i start, Vec2i goal, float epsilon);
};
