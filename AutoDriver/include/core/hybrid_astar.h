#pragma once

#include <vector>
#include <memory>
#include "grid.h"
#include "vec2.h"

/**
 * Motion primitive for Hybrid A* - represents a possible vehicle motion.
 * Includes position, heading, and steering angle.
 */
struct MotionPrimitive {
    float delta_x;      // Forward displacement
    float delta_y;      // Lateral displacement
    float delta_theta;  // Change in heading (radians)
    float cost;         // Cost of this motion
    float steering;     // Steering angle (radians)
    
    MotionPrimitive(float dx, float dy, float dtheta, float c, float s = 0.0f)
        : delta_x(dx), delta_y(dy), delta_theta(dtheta), cost(c), steering(s) {}
};

/**
 * State in Hybrid A* - includes position and heading.
 */
struct HybridState {
    Vec2 pos;           // Position (continuous)
    float theta;        // Heading angle (radians)
    HybridState* parent;
    float g_cost;
    float h_cost;
    int steering_direction; // -1 left, 0 straight, 1 right
    
    HybridState(Vec2 p, float t, HybridState* par = nullptr)
        : pos(p), theta(t), parent(par), g_cost(0.0f), h_cost(0.0f), steering_direction(0) {}
    
    float f_cost() const { return g_cost + h_cost; }
};

/**
 * Vehicle parameters for kinematic constraints.
 */
struct VehicleParams {
    float length;           // Vehicle length (wheelbase)
    float width;            // Vehicle width
    float min_turn_radius;  // Minimum turning radius
    float max_steering;     // Maximum steering angle (radians)
    float speed;            // Forward speed (for motion primitives)
    
    VehicleParams()
        : length(4.0f), width(2.0f), min_turn_radius(5.0f),
          max_steering(0.6f), speed(1.0f) {}
};

/**
 * Result of Hybrid A* search.
 */
struct HybridAStarResult {
    std::vector<HybridState> path;
    std::vector<Vec2> explored;
    int nodes_expanded;
    float path_cost;
    bool success;
    
    HybridAStarResult() : nodes_expanded(0), path_cost(0.0f), success(false) {}
};

/**
 * Hybrid A* - path planning with kinematic constraints.
 * Unlike regular A*, this considers vehicle heading and turning radius.
 */
class HybridAStar {
public:
    explicit HybridAStar(const Grid& grid, const VehicleParams& params = VehicleParams());
    
    // Main planning function
    HybridAStarResult findPath(Vec2 start, float start_theta,
                               Vec2 goal, float goal_theta,
                               int max_iterations = 10000);
    
    // Configuration
    void setVehicleParams(const VehicleParams& params) { vehicle_params_ = params; }
    void setAngularResolution(int divisions) { angular_divisions_ = divisions; }
    
private:
    const Grid& grid_;
    VehicleParams vehicle_params_;
    int angular_divisions_;  // Number of angle divisions (e.g., 72 = 5° resolution)
    
    std::vector<MotionPrimitive> motion_primitives_;
    
    // Generate motion primitives based on vehicle parameters
    void generateMotionPrimitives();
    
    // Check if vehicle footprint collides with obstacles
    bool isCollisionFree(Vec2 pos, float theta) const;
    
    // Check if path between two states is collision-free
    bool isPathCollisionFree(const HybridState& from, const HybridState& to) const;
    
    // Heuristic for Hybrid A* (considers heading)
    float calculateHeuristic(Vec2 pos, float theta, Vec2 goal, float goal_theta) const;
    
    // Apply motion primitive to get next state
    HybridState applyMotion(const HybridState& state, const MotionPrimitive& motion) const;
    
    // Get discrete cell index for state space
    int getStateIndex(Vec2 pos, float theta) const;
    
    // Reconstruct path from goal to start
    std::vector<HybridState> reconstructPath(HybridState* goal) const;
};
