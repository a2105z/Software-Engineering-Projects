#pragma once

#include <vector>
#include <unordered_set>
#include <memory>
#include "grid.h"
#include "node.h"
#include "vec2.h"

/**
 * Hash function for Vec2i to use in unordered_set.
 */
struct Vec2iHash {
    std::size_t operator()(const Vec2i& v) const {
        return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
    }
};

/**
 * Result of A* search including path and search statistics.
 */
struct AStarResult {
    std::vector<Vec2i> path;
    std::vector<Vec2i> visited;    // Closed set (for visualization)
    std::vector<Vec2i> explored;   // Open set at each step (for visualization)
    int nodes_expanded;
    float path_cost;
    bool success;
    
    AStarResult() : nodes_expanded(0), path_cost(0.0f), success(false) {}
};

/**
 * A* pathfinding algorithm.
 */
class AStar {
public:
    explicit AStar(const Grid& grid);
    
    // Find path from start to goal
    AStarResult findPath(Vec2i start, Vec2i goal);
    
    // Heuristic functions
    static float euclideanDistance(Vec2i a, Vec2i b);
    static float manhattanDistance(Vec2i a, Vec2i b);
    
private:
    const Grid& grid_;
    
    std::vector<Vec2i> getNeighbors(Vec2i pos) const;
    std::vector<Vec2i> reconstructPath(Node* goal) const;
};
