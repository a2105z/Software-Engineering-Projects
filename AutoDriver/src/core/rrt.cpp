#include "core/rrt.h"
#include <cmath>
#include <algorithm>
#include <limits>

// ============================================================================
// RRT Implementation
// ============================================================================

RRT::RRT(const Grid& grid)
    : grid_(grid)
    , step_size_(1.0f)
    , goal_bias_(0.1f)
    , goal_threshold_(1.0f)
    , rng_(std::random_device{}())
    , dist_x_(0.0f, static_cast<float>(grid.getWidth()))
    , dist_y_(0.0f, static_cast<float>(grid.getHeight()))
    , dist_prob_(0.0f, 1.0f) {
}

RRT::~RRT() = default;

RRTResult RRT::findPath(Vec2 start, Vec2 goal, int max_iterations) {
    RRTResult result;
    nodes_.clear();
    
    // Validate start and goal
    if (!isInBounds(start) || !isInBounds(goal)) {
        return result;
    }
    
    // Initialize tree with start node
    auto start_node = std::make_unique<RRTNode>(start, nullptr, 0.0f);
    RRTNode* start_ptr = start_node.get();
    nodes_.push_back(std::move(start_node));
    
    RRTNode* best_node = nullptr;
    float best_distance = std::numeric_limits<float>::max();
    
    for (int iter = 0; iter < max_iterations; iter++) {
        // Sample random point (with goal bias)
        Vec2 sample = sampleRandom();
        if (dist_prob_(rng_) < goal_bias_) {
            sample = goal;
        }
        
        // Find nearest node in tree
        RRTNode* nearest = findNearest(sample);
        if (!nearest) continue;
        
        // Steer towards sample
        Vec2 new_pos = steer(nearest->pos, sample);
        
        // Check collision
        if (!isCollisionFree(nearest->pos, new_pos)) {
            continue;
        }
        
        // Add new node to tree
        RRTNode* new_node = addNode(new_pos, nearest);
        
        // Check if goal is reached
        float dist_to_goal = distance(new_pos, goal);
        if (dist_to_goal < goal_threshold_) {
            // Found path to goal
            result.success = true;
            result.iterations = iter + 1;
            result.path = reconstructPath(new_node);
            result.path.push_back(goal);  // Add actual goal
            result.path_cost = new_node->cost + dist_to_goal;
            
            // Collect all tree nodes for visualization
            for (const auto& node : nodes_) {
                result.tree_nodes.push_back(node->pos);
            }
            
            return result;
        }
        
        // Track closest node to goal
        if (dist_to_goal < best_distance) {
            best_distance = dist_to_goal;
            best_node = new_node;
        }
    }
    
    // Max iterations reached - return path to closest node
    result.iterations = max_iterations;
    if (best_node) {
        result.path = reconstructPath(best_node);
        result.path_cost = best_node->cost;
    }
    
    // Collect tree for visualization
    for (const auto& node : nodes_) {
        result.tree_nodes.push_back(node->pos);
    }
    
    return result;
}

Vec2 RRT::sampleRandom() {
    return Vec2(dist_x_(rng_), dist_y_(rng_));
}

RRTNode* RRT::findNearest(Vec2 sample) {
    RRTNode* nearest = nullptr;
    float min_dist = std::numeric_limits<float>::max();
    
    for (const auto& node : nodes_) {
        float dist = distance(node->pos, sample);
        if (dist < min_dist) {
            min_dist = dist;
            nearest = node.get();
        }
    }
    
    return nearest;
}

Vec2 RRT::steer(Vec2 from, Vec2 to) {
    Vec2 direction = to - from;
    float dist = direction.length();
    
    if (dist <= step_size_) {
        return to;
    }
    
    // Normalize and scale to step_size
    return from + direction * (step_size_ / dist);
}

bool RRT::isCollisionFree(Vec2 from, Vec2 to) {
    // Check line segment from->to for collisions
    Vec2 direction = to - from;
    float dist = direction.length();
    
    if (dist < 0.01f) return true;
    
    // Sample points along the line
    int num_checks = static_cast<int>(std::ceil(dist * 2.0f));  // 2 checks per unit
    for (int i = 0; i <= num_checks; i++) {
        float t = static_cast<float>(i) / num_checks;
        Vec2 point = from + direction * t;
        
        // Convert to grid coordinates
        int gx = static_cast<int>(std::round(point.x));
        int gy = static_cast<int>(std::round(point.y));
        
        if (grid_.isObstacle(gx, gy)) {
            return false;
        }
    }
    
    return true;
}

RRTNode* RRT::addNode(Vec2 pos, RRTNode* parent) {
    float cost = parent ? parent->cost + distance(parent->pos, pos) : 0.0f;
    auto node = std::make_unique<RRTNode>(pos, parent, cost);
    RRTNode* node_ptr = node.get();
    
    if (parent) {
        parent->children.push_back(node_ptr);
    }
    
    nodes_.push_back(std::move(node));
    return node_ptr;
}

std::vector<Vec2> RRT::reconstructPath(RRTNode* goal) {
    std::vector<Vec2> path;
    RRTNode* current = goal;
    
    while (current != nullptr) {
        path.push_back(current->pos);
        current = current->parent;
    }
    
    std::reverse(path.begin(), path.end());
    return path;
}

float RRT::distance(Vec2 a, Vec2 b) const {
    return a.distanceTo(b);
}

bool RRT::isInBounds(Vec2 pos) const {
    return pos.x >= 0 && pos.x < grid_.getWidth() &&
           pos.y >= 0 && pos.y < grid_.getHeight();
}

// ============================================================================
// RRT* Implementation
// ============================================================================

RRTStar::RRTStar(const Grid& grid)
    : RRT(grid)
    , rewire_radius_(3.0f) {
}

RRTResult RRTStar::findPath(Vec2 start, Vec2 goal, int max_iterations) {
    RRTResult result;
    nodes_.clear();
    
    // Validate start and goal
    if (!isInBounds(start) || !isInBounds(goal)) {
        return result;
    }
    
    // Initialize tree with start node
    auto start_node = std::make_unique<RRTNode>(start, nullptr, 0.0f);
    nodes_.push_back(std::move(start_node));
    
    RRTNode* best_node = nullptr;
    float best_distance = std::numeric_limits<float>::max();
    
    for (int iter = 0; iter < max_iterations; iter++) {
        // Sample random point (with goal bias)
        Vec2 sample = sampleRandom();
        if (dist_prob_(rng_) < goal_bias_) {
            sample = goal;
        }
        
        // Find nearest node
        RRTNode* nearest = findNearest(sample);
        if (!nearest) continue;
        
        // Steer towards sample
        Vec2 new_pos = steer(nearest->pos, sample);
        
        // Check collision
        if (!isCollisionFree(nearest->pos, new_pos)) {
            continue;
        }
        
        // Find nearby nodes for RRT*
        std::vector<RRTNode*> nearby = findNearby(new_pos, rewire_radius_);
        
        // Choose best parent
        RRTNode* best_parent = chooseBestParent(new_pos, nearby);
        if (!best_parent) best_parent = nearest;
        
        // Add new node
        RRTNode* new_node = addNode(new_pos, best_parent);
        
        // Rewire tree
        rewire(new_node, nearby);
        
        // Check if goal is reached
        float dist_to_goal = distance(new_pos, goal);
        if (dist_to_goal < goal_threshold_) {
            // Check if path to goal is collision-free
            if (isCollisionFree(new_pos, goal)) {
                result.success = true;
                result.iterations = iter + 1;
                result.path = reconstructPath(new_node);
                result.path.push_back(goal);
                result.path_cost = new_node->cost + dist_to_goal;
                
                // Collect tree for visualization
                for (const auto& node : nodes_) {
                    result.tree_nodes.push_back(node->pos);
                }
                
                return result;
            }
        }
        
        // Track closest node
        if (dist_to_goal < best_distance) {
            best_distance = dist_to_goal;
            best_node = new_node;
        }
    }
    
    // Max iterations reached
    result.iterations = max_iterations;
    if (best_node) {
        result.path = reconstructPath(best_node);
        result.path_cost = best_node->cost;
    }
    
    // Collect tree
    for (const auto& node : nodes_) {
        result.tree_nodes.push_back(node->pos);
    }
    
    return result;
}

std::vector<RRTNode*> RRTStar::findNearby(Vec2 pos, float radius) {
    std::vector<RRTNode*> nearby;
    
    for (const auto& node : nodes_) {
        if (distance(node->pos, pos) < radius) {
            nearby.push_back(node.get());
        }
    }
    
    return nearby;
}

RRTNode* RRTStar::chooseBestParent(Vec2 pos, const std::vector<RRTNode*>& nearby) {
    RRTNode* best_parent = nullptr;
    float best_cost = std::numeric_limits<float>::max();
    
    for (RRTNode* node : nearby) {
        float cost = node->cost + distance(node->pos, pos);
        
        if (cost < best_cost && isCollisionFree(node->pos, pos)) {
            best_cost = cost;
            best_parent = node;
        }
    }
    
    return best_parent;
}

void RRTStar::rewire(RRTNode* new_node, const std::vector<RRTNode*>& nearby) {
    for (RRTNode* node : nearby) {
        if (node == new_node || node == new_node->parent) continue;
        
        float new_cost = new_node->cost + distance(new_node->pos, node->pos);
        
        if (new_cost < node->cost && isCollisionFree(new_node->pos, node->pos)) {
            // Remove node from old parent's children
            if (node->parent) {
                auto& children = node->parent->children;
                children.erase(std::remove(children.begin(), children.end(), node), 
                              children.end());
            }
            
            // Update parent and cost
            node->parent = new_node;
            node->cost = new_cost;
            new_node->children.push_back(node);
            
            // Recursively update costs of descendants
            std::vector<RRTNode*> to_update = {node};
            while (!to_update.empty()) {
                RRTNode* current = to_update.back();
                to_update.pop_back();
                
                for (RRTNode* child : current->children) {
                    child->cost = current->cost + distance(current->pos, child->pos);
                    to_update.push_back(child);
                }
            }
        }
    }
}
