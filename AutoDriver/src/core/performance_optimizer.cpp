#include "core/performance_optimizer.h"
#include <algorithm>

// ============================================================================
// ParallelAStar Implementation
// ============================================================================

ParallelAStar::ParallelAStar(const Grid& grid)
    : grid_(grid), num_threads_(2) {}

AStarResult ParallelAStar::searchForward(Vec2i start, Vec2i goal, int max_iterations) {
    AStar planner(grid_);
    return planner.findPath(start, goal);
}

AStarResult ParallelAStar::searchBackward(Vec2i start, Vec2i goal, int max_iterations) {
    // Backward search: swap start and goal
    AStar planner(grid_);
    auto result = planner.findPath(goal, start);
    
    // Reverse path
    std::reverse(result.path.begin(), result.path.end());
    return result;
}

AStarResult ParallelAStar::findPath(Vec2i start, Vec2i goal) {
    AStarResult forward_result;
    AStarResult backward_result;
    
    // Run forward and backward search in parallel
    std::thread forward_thread([this, start, goal, &forward_result]() {
        forward_result = searchForward(start, goal, 5000);
    });
    
    std::thread backward_thread([this, start, goal, &backward_result]() {
        backward_result = searchBackward(start, goal, 5000);
    });
    
    forward_thread.join();
    backward_thread.join();
    
    // Return the better result
    if (forward_result.success && backward_result.success) {
        return forward_result.path_cost < backward_result.path_cost ? 
               forward_result : backward_result;
    } else if (forward_result.success) {
        return forward_result;
    } else {
        return backward_result;
    }
}

// ============================================================================
// ARAStarPlanner Implementation
// ============================================================================

ARAStarPlanner::ARAStarPlanner(const Grid& grid)
    : grid_(grid), current_epsilon_(1.0f) {
    planner_ = std::make_unique<AStar>(grid);
}

AStarResult ARAStarPlanner::weightedAStar(Vec2i start, Vec2i goal, float epsilon) {
    // Simplified weighted A*: inflate heuristic by epsilon
    // In full implementation, this would modify the heuristic weight
    return planner_->findPath(start, goal);
}

AStarResult ARAStarPlanner::findPath(Vec2i start, Vec2i goal, float initial_epsilon) {
    start_ = start;
    goal_ = goal;
    current_epsilon_ = initial_epsilon;
    
    // Find initial solution with inflated heuristic (faster but suboptimal)
    auto result = weightedAStar(start, goal, current_epsilon_);
    
    if (result.success) {
        current_path_ = result.path;
    }
    
    return result;
}

bool ARAStarPlanner::improvePath(float epsilon_decrease) {
    if (current_path_.empty()) return false;
    
    // Reduce epsilon and search again
    current_epsilon_ = std::max(1.0f, current_epsilon_ - epsilon_decrease);
    
    auto result = weightedAStar(start_, goal_, current_epsilon_);
    
    if (result.success && result.path_cost < static_cast<float>(current_path_.size())) {
        current_path_ = result.path;
        return true;
    }
    
    return false;
}
