#include "core/hybrid_astar.h"
#include <cmath>
#include <queue>
#include <unordered_map>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper for priority queue comparison
struct HybridStateCompare {
    bool operator()(const HybridState* a, const HybridState* b) const {
        return a->f_cost() > b->f_cost();
    }
};

HybridAStar::HybridAStar(const Grid& grid, const VehicleParams& params)
    : grid_(grid), vehicle_params_(params), angular_divisions_(72) {
    generateMotionPrimitives();
}

void HybridAStar::generateMotionPrimitives() {
    motion_primitives_.clear();
    
    // Generate primitives for different steering angles
    const int num_steering_angles = 5;
    const float max_steer = vehicle_params_.max_steering;
    
    for (int i = 0; i < num_steering_angles; i++) {
        float steering = -max_steer + (2.0f * max_steer * i) / (num_steering_angles - 1);
        
        // Calculate motion based on bicycle model
        float dt = 1.0f;  // Time step
        float v = vehicle_params_.speed;
        float L = vehicle_params_.length;
        
        // Bicycle kinematic model
        float dx = v * dt * std::cos(steering);
        float dy = v * dt * std::sin(steering);
        float dtheta = (v * dt * std::tan(steering)) / L;
        
        // Cost includes distance and penalizes steering
        float cost = std::sqrt(dx * dx + dy * dy) + 0.1f * std::abs(steering);
        
        motion_primitives_.emplace_back(dx, dy, dtheta, cost, steering);
    }
    
    // Add reverse motion (for parking scenarios)
    float dx_rev = -vehicle_params_.speed * 0.5f;
    motion_primitives_.emplace_back(dx_rev, 0.0f, 0.0f, std::abs(dx_rev) * 1.5f, 0.0f);
}

bool HybridAStar::isCollisionFree(Vec2 pos, float theta) const {
    // Check vehicle footprint (simplified as rectangle)
    float half_length = vehicle_params_.length / 2.0f;
    float half_width = vehicle_params_.width / 2.0f;
    
    // Sample points around vehicle perimeter
    std::vector<Vec2> corners = {
        Vec2(half_length, half_width),
        Vec2(half_length, -half_width),
        Vec2(-half_length, half_width),
        Vec2(-half_length, -half_width)
    };
    
    for (const auto& corner : corners) {
        // Rotate corner by theta
        float cos_theta = std::cos(theta);
        float sin_theta = std::sin(theta);
        Vec2 rotated(
            corner.x * cos_theta - corner.y * sin_theta,
            corner.x * sin_theta + corner.y * cos_theta
        );
        
        Vec2 world_pos = pos + rotated;
        int gx = static_cast<int>(std::round(world_pos.x));
        int gy = static_cast<int>(std::round(world_pos.y));
        
        if (grid_.isObstacle(gx, gy)) {
            return false;
        }
    }
    
    return true;
}

bool HybridAStar::isPathCollisionFree(const HybridState& from, const HybridState& to) const {
    // Sample along path
    int num_samples = 5;
    for (int i = 0; i <= num_samples; i++) {
        float t = static_cast<float>(i) / num_samples;
        Vec2 pos = from.pos + (to.pos - from.pos) * t;
        float theta = from.theta + (to.theta - from.theta) * t;
        
        if (!isCollisionFree(pos, theta)) {
            return false;
        }
    }
    
    return true;
}

float HybridAStar::calculateHeuristic(Vec2 pos, float theta, Vec2 goal, float goal_theta) const {
    // Euclidean distance to goal
    float dist = pos.distanceTo(goal);
    
    // Add heading difference penalty
    float angle_diff = std::abs(theta - goal_theta);
    while (angle_diff > M_PI) angle_diff -= 2.0f * M_PI;
    angle_diff = std::abs(angle_diff);
    
    return dist + angle_diff * vehicle_params_.min_turn_radius * 0.5f;
}

HybridState HybridAStar::applyMotion(const HybridState& state, const MotionPrimitive& motion) const {
    // Apply motion in vehicle's local frame
    float cos_theta = std::cos(state.theta);
    float sin_theta = std::sin(state.theta);
    
    Vec2 new_pos(
        state.pos.x + motion.delta_x * cos_theta - motion.delta_y * sin_theta,
        state.pos.y + motion.delta_x * sin_theta + motion.delta_y * cos_theta
    );
    
    float new_theta = state.theta + motion.delta_theta;
    
    // Normalize angle to [-π, π]
    while (new_theta > M_PI) new_theta -= 2.0f * M_PI;
    while (new_theta < -M_PI) new_theta += 2.0f * M_PI;
    
    HybridState new_state(new_pos, new_theta);
    return new_state;
}

int HybridAStar::getStateIndex(Vec2 pos, float theta) const {
    int x_idx = static_cast<int>(pos.x);
    int y_idx = static_cast<int>(pos.y);
    int theta_idx = static_cast<int>((theta + M_PI) / (2.0f * M_PI) * angular_divisions_);
    
    return (x_idx * grid_.getHeight() + y_idx) * angular_divisions_ + theta_idx;
}

std::vector<HybridState> HybridAStar::reconstructPath(HybridState* goal) const {
    std::vector<HybridState> path;
    HybridState* current = goal;
    
    while (current != nullptr) {
        path.push_back(*current);
        current = current->parent;
    }
    
    std::reverse(path.begin(), path.end());
    return path;
}

HybridAStarResult HybridAStar::findPath(Vec2 start, float start_theta,
                                       Vec2 goal, float goal_theta,
                                       int max_iterations) {
    HybridAStarResult result;
    
    // Validate start and goal
    if (!isCollisionFree(start, start_theta) || !isCollisionFree(goal, goal_theta)) {
        return result;
    }
    
    // Priority queue and visited set
    std::priority_queue<HybridState*, std::vector<HybridState*>, HybridStateCompare> open_set;
    std::unordered_map<int, std::unique_ptr<HybridState>> all_states;
    std::unordered_map<int, float> best_costs;
    
    // Initialize start state
    auto start_state = std::make_unique<HybridState>(start, start_theta);
    start_state->g_cost = 0.0f;
    start_state->h_cost = calculateHeuristic(start, start_theta, goal, goal_theta);
    
    int start_idx = getStateIndex(start, start_theta);
    HybridState* start_ptr = start_state.get();
    all_states[start_idx] = std::move(start_state);
    open_set.push(start_ptr);
    best_costs[start_idx] = 0.0f;
    
    int iterations = 0;
    
    while (!open_set.empty() && iterations < max_iterations) {
        iterations++;
        
        HybridState* current = open_set.top();
        open_set.pop();
        
        // Check if goal reached
        float dist_to_goal = current->pos.distanceTo(goal);
        float angle_diff = std::abs(current->theta - goal_theta);
        while (angle_diff > M_PI) angle_diff -= 2.0f * M_PI;
        angle_diff = std::abs(angle_diff);
        
        if (dist_to_goal < 1.0f && angle_diff < 0.2f) {
            result.success = true;
            result.path = reconstructPath(current);
            result.nodes_expanded = iterations;
            result.path_cost = current->g_cost;
            return result;
        }
        
        // Expand neighbors using motion primitives
        for (const auto& motion : motion_primitives_) {
            HybridState next = applyMotion(*current, motion);
            
            // Check bounds
            if (next.pos.x < 0 || next.pos.x >= grid_.getWidth() ||
                next.pos.y < 0 || next.pos.y >= grid_.getHeight()) {
                continue;
            }
            
            // Check collision
            if (!isCollisionFree(next.pos, next.theta)) {
                continue;
            }
            
            if (!isPathCollisionFree(*current, next)) {
                continue;
            }
            
            float new_g_cost = current->g_cost + motion.cost;
            int next_idx = getStateIndex(next.pos, next.theta);
            
            // Check if this is a better path
            if (best_costs.find(next_idx) == best_costs.end() ||
                new_g_cost < best_costs[next_idx]) {
                
                auto next_state = std::make_unique<HybridState>(next.pos, next.theta, current);
                next_state->g_cost = new_g_cost;
                next_state->h_cost = calculateHeuristic(next.pos, next.theta, goal, goal_theta);
                
                HybridState* next_ptr = next_state.get();
                all_states[next_idx] = std::move(next_state);
                open_set.push(next_ptr);
                best_costs[next_idx] = new_g_cost;
                
                result.explored.push_back(next_ptr->pos);
            }
        }
    }
    
    result.nodes_expanded = iterations;
    return result;
}
