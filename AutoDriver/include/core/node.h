#pragma once

#include "vec2.h"

/**
 * Node for A* pathfinding algorithm.
 */
struct Node {
    Vec2i pos;           // Grid position
    float g_cost;        // Cost from start
    float h_cost;        // Heuristic cost to goal
    Node* parent;        // Parent node for path reconstruction
    
    Node(Vec2i pos, float g, float h, Node* parent = nullptr)
        : pos(pos), g_cost(g), h_cost(h), parent(parent) {}
    
    float f_cost() const {
        return g_cost + h_cost;
    }
    
    // For priority queue (min-heap)
    bool operator>(const Node& other) const {
        return f_cost() > other.f_cost();
    }
};
