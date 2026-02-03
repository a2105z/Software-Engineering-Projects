#pragma once

#include <vector>
#include <memory>
#include "vec2.h"
#include "astar.h"

/**
 * Agent in multi-agent simulation.
 */
struct Agent {
    int id;
    Vec2 position;
    Vec2 goal;
    Vec2 velocity;
    float radius;
    std::vector<Vec2> planned_path;
    int path_index;
    
    Agent(int agent_id, Vec2 pos, Vec2 g, float r = 0.5f)
        : id(agent_id), position(pos), goal(g), velocity(0, 0), 
          radius(r), path_index(0) {}
};

/**
 * Multi-agent coordinator for collision-free path planning.
 */
class MultiAgentPlanner {
public:
    explicit MultiAgentPlanner(const Grid& grid);
    
    // Add agent to simulation
    void addAgent(const Agent& agent);
    
    // Plan paths for all agents with collision avoidance
    void planPaths();
    
    // Update agent positions (one timestep)
    void update(float dt);
    
    // Check for potential collisions
    bool checkCollision(int agent1, int agent2, float time_horizon = 2.0f) const;
    
    // Get all agents
    const std::vector<Agent>& getAgents() const { return agents_; }
    
    // Replan for specific agent (e.g., after collision detected)
    void replanAgent(int agent_id);
    
private:
    const Grid& grid_;
    std::vector<Agent> agents_;
    std::unique_ptr<AStar> planner_;
    
    // Check if path is collision-free with other agents
    bool isPathSafe(const std::vector<Vec2i>& path, int agent_id) const;
    
    // Apply velocity obstacle avoidance
    Vec2 calculateAvoidanceVelocity(int agent_id) const;
};
