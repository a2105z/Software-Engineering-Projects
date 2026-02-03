#include "core/astar.h"
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <cmath>

AStar::AStar(const Grid& grid) : grid_(grid) {}

AStarResult AStar::findPath(Vec2i start, Vec2i goal) {
    AStarResult result;
    
    // Validate start and goal
    if (!grid_.isValid(start.x, start.y) || grid_.isObstacle(start.x, start.y) ||
        !grid_.isValid(goal.x, goal.y) || grid_.isObstacle(goal.x, goal.y)) {
        return result;  // Invalid positions
    }
    
    // Priority queue for open set (min-heap by f_cost)
    auto cmp = [](Node* a, Node* b) { return *a > *b; };
    std::priority_queue<Node*, std::vector<Node*>, decltype(cmp)> open_set(cmp);
    
    // Track visited nodes
    std::unordered_set<Vec2i, Vec2iHash> closed_set;
    
    // Map from position to node (for finding existing nodes)
    std::unordered_map<Vec2i, Node*, Vec2iHash> node_map;
    
    // Storage for all allocated nodes (for cleanup)
    std::vector<std::unique_ptr<Node>> all_nodes;
    
    // Create start node
    auto start_node = std::make_unique<Node>(
        start, 
        0.0f, 
        euclideanDistance(start, goal),
        nullptr
    );
    Node* start_ptr = start_node.get();
    node_map[start] = start_ptr;
    open_set.push(start_ptr);
    all_nodes.push_back(std::move(start_node));
    
    Node* goal_node = nullptr;
    
    while (!open_set.empty()) {
        Node* current = open_set.top();
        open_set.pop();
        
        // Skip if already visited
        if (closed_set.count(current->pos)) {
            continue;
        }
        
        // Mark as visited
        closed_set.insert(current->pos);
        result.visited.push_back(current->pos);
        result.nodes_expanded++;
        
        // Check if we reached the goal
        if (current->pos == goal) {
            goal_node = current;
            result.success = true;
            result.path_cost = current->g_cost;
            break;
        }
        
        // Explore neighbors
        for (const Vec2i& neighbor_pos : getNeighbors(current->pos)) {
            if (closed_set.count(neighbor_pos)) {
                continue;
            }
            
            // Calculate costs
            float tentative_g = current->g_cost + 1.0f;  // Uniform cost
            
            // Check if we already have this neighbor in open set
            auto it = node_map.find(neighbor_pos);
            if (it != node_map.end()) {
                // Update if we found a better path
                if (tentative_g < it->second->g_cost) {
                    it->second->g_cost = tentative_g;
                    it->second->parent = current;
                }
            } else {
                // Create new node
                auto neighbor_node = std::make_unique<Node>(
                    neighbor_pos,
                    tentative_g,
                    euclideanDistance(neighbor_pos, goal),
                    current
                );
                Node* neighbor_ptr = neighbor_node.get();
                node_map[neighbor_pos] = neighbor_ptr;
                open_set.push(neighbor_ptr);
                all_nodes.push_back(std::move(neighbor_node));
                result.explored.push_back(neighbor_pos);
            }
        }
    }
    
    // Reconstruct path if goal was reached
    if (goal_node) {
        result.path = reconstructPath(goal_node);
    }
    
    return result;
}

std::vector<Vec2i> AStar::getNeighbors(Vec2i pos) const {
    std::vector<Vec2i> neighbors;
    
    // 4-directional movement (up, down, left, right)
    const int dx[] = {0, 0, -1, 1};
    const int dy[] = {-1, 1, 0, 0};
    
    for (int i = 0; i < 4; i++) {
        int nx = pos.x + dx[i];
        int ny = pos.y + dy[i];
        
        if (grid_.isValid(nx, ny) && !grid_.isObstacle(nx, ny)) {
            neighbors.push_back(Vec2i(nx, ny));
        }
    }
    
    return neighbors;
}

std::vector<Vec2i> AStar::reconstructPath(Node* goal) const {
    std::vector<Vec2i> path;
    Node* current = goal;
    
    while (current != nullptr) {
        path.push_back(current->pos);
        current = current->parent;
    }
    
    std::reverse(path.begin(), path.end());
    return path;
}

float AStar::euclideanDistance(Vec2i a, Vec2i b) {
    float dx = static_cast<float>(a.x - b.x);
    float dy = static_cast<float>(a.y - b.y);
    return std::sqrt(dx * dx + dy * dy);
}

float AStar::manhattanDistance(Vec2i a, Vec2i b) {
    return static_cast<float>(std::abs(a.x - b.x) + std::abs(a.y - b.y));
}
