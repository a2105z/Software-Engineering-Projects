#pragma once

#include <vector>
#include <memory>
#include <random>
#include "grid.h"
#include "vec2.h"

/**
 * Node for RRT tree structure.
 */
struct RRTNode {
    Vec2 pos;              // Continuous position (not grid-based)
    RRTNode* parent;       // Parent node in tree
    float cost;            // Cost from root (for RRT*)
    std::vector<RRTNode*> children;  // Children (for rewiring in RRT*)
    
    RRTNode(Vec2 pos, RRTNode* parent = nullptr, float cost = 0.0f)
        : pos(pos), parent(parent), cost(cost) {}
};

/**
 * Result of RRT search.
 */
struct RRTResult {
    std::vector<Vec2> path;
    std::vector<Vec2> tree_nodes;  // All nodes in tree (for visualization)
    int iterations;
    float path_cost;
    bool success;
    
    RRTResult() : iterations(0), path_cost(0.0f), success(false) {}
};

/**
 * RRT (Rapidly-exploring Random Trees) pathfinding.
 * Good for high-dimensional spaces and complex environments.
 */
class RRT {
public:
    explicit RRT(const Grid& grid);
    virtual ~RRT();
    
    // Main planning function
    RRTResult findPath(Vec2 start, Vec2 goal, int max_iterations = 5000);
    
    // Configuration
    void setStepSize(float step_size) { step_size_ = step_size; }
    void setGoalBias(float bias) { goal_bias_ = bias; }
    void setGoalThreshold(float threshold) { goal_threshold_ = threshold; }
    
protected:
    const Grid& grid_;
    float step_size_;           // Maximum step distance
    float goal_bias_;           // Probability of sampling goal
    float goal_threshold_;      // Distance to consider goal reached
    
    std::mt19937 rng_;
    std::uniform_real_distribution<float> dist_x_;
    std::uniform_real_distribution<float> dist_y_;
    std::uniform_real_distribution<float> dist_prob_;
    
    std::vector<std::unique_ptr<RRTNode>> nodes_;
    
    // Core RRT operations
    virtual Vec2 sampleRandom();
    virtual RRTNode* findNearest(Vec2 sample);
    virtual Vec2 steer(Vec2 from, Vec2 to);
    virtual bool isCollisionFree(Vec2 from, Vec2 to);
    virtual RRTNode* addNode(Vec2 pos, RRTNode* parent);
    virtual std::vector<Vec2> reconstructPath(RRTNode* goal);
    
    // Helper functions
    float distance(Vec2 a, Vec2 b) const;
    bool isInBounds(Vec2 pos) const;
};

/**
 * RRT* (optimal RRT) - extends RRT with rewiring for optimal paths.
 */
class RRTStar : public RRT {
public:
    explicit RRTStar(const Grid& grid);
    
    RRTResult findPath(Vec2 start, Vec2 goal, int max_iterations = 5000);
    
    void setRewireRadius(float radius) { rewire_radius_ = radius; }
    
private:
    float rewire_radius_;  // Radius for finding nearby nodes to rewire
    
    // RRT* specific operations
    std::vector<RRTNode*> findNearby(Vec2 pos, float radius);
    RRTNode* chooseBestParent(Vec2 pos, const std::vector<RRTNode*>& nearby);
    void rewire(RRTNode* new_node, const std::vector<RRTNode*>& nearby);
};
