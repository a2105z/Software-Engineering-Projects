#include "core/multi_agent.h"
#include <cmath>
#include <algorithm>

MultiAgentPlanner::MultiAgentPlanner(const Grid& grid)
    : grid_(grid) {
    planner_ = std::make_unique<AStar>(grid);
}

void MultiAgentPlanner::addAgent(const Agent& agent) {
    agents_.push_back(agent);
}

void MultiAgentPlanner::planPaths() {
    // Plan path for each agent
    for (auto& agent : agents_) {
        Vec2i start(static_cast<int>(agent.position.x), static_cast<int>(agent.position.y));
        Vec2i goal(static_cast<int>(agent.goal.x), static_cast<int>(agent.goal.y));
        
        auto result = planner_->findPath(start, goal);
        
        if (result.success) {
            agent.planned_path.clear();
            for (const auto& p : result.path) {
                agent.planned_path.emplace_back(static_cast<float>(p.x), static_cast<float>(p.y));
            }
            agent.path_index = 0;
        }
    }
}

bool MultiAgentPlanner::checkCollision(int agent1, int agent2, float time_horizon) const {
    if (agent1 >= static_cast<int>(agents_.size()) || 
        agent2 >= static_cast<int>(agents_.size())) {
        return false;
    }
    
    const auto& a1 = agents_[agent1];
    const auto& a2 = agents_[agent2];
    
    // Simple distance check
    float dist = a1.position.distanceTo(a2.position);
    float collision_dist = a1.radius + a2.radius + 0.5f;
    
    return dist < collision_dist;
}

bool MultiAgentPlanner::isPathSafe(const std::vector<Vec2i>& path, int agent_id) const {
    // Simplified safety check: ensure path doesn't intersect with other agents' current positions
    const auto& agent = agents_[agent_id];
    
    for (size_t i = 0; i < agents_.size(); i++) {
        if (static_cast<int>(i) == agent_id) continue;
        
        Vec2 other_pos = agents_[i].position;
        for (const auto& wp : path) {
            Vec2 wp_f(static_cast<float>(wp.x), static_cast<float>(wp.y));
            if (wp_f.distanceTo(other_pos) < agent.radius + agents_[i].radius + 1.0f) {
                return false;
            }
        }
    }
    
    return true;
}

Vec2 MultiAgentPlanner::calculateAvoidanceVelocity(int agent_id) const {
    if (agent_id >= static_cast<int>(agents_.size())) {
        return Vec2(0, 0);
    }
    
    const auto& agent = agents_[agent_id];
    Vec2 desired_velocity = agent.velocity;
    
    // Simple repulsion from nearby agents
    for (size_t i = 0; i < agents_.size(); i++) {
        if (static_cast<int>(i) == agent_id) continue;
        
        Vec2 diff = agent.position - agents_[i].position;
        float dist = diff.length();
        
        if (dist < 3.0f && dist > 0.01f) {
            // Add repulsive force
            desired_velocity = desired_velocity + diff * (1.0f / dist) * 0.5f;
        }
    }
    
    // Normalize velocity
    float len = desired_velocity.length();
    if (len > 1.0f) {
        desired_velocity = desired_velocity * (1.0f / len);
    }
    
    return desired_velocity;
}

void MultiAgentPlanner::replanAgent(int agent_id) {
    if (agent_id < 0 || agent_id >= static_cast<int>(agents_.size())) {
        return;
    }
    
    auto& agent = agents_[agent_id];
    Vec2i start(static_cast<int>(agent.position.x), static_cast<int>(agent.position.y));
    Vec2i goal(static_cast<int>(agent.goal.x), static_cast<int>(agent.goal.y));
    
    auto result = planner_->findPath(start, goal);
    
    if (result.success) {
        agent.planned_path.clear();
        for (const auto& p : result.path) {
            agent.planned_path.emplace_back(static_cast<float>(p.x), static_cast<float>(p.y));
        }
        agent.path_index = 0;
    }
}

void MultiAgentPlanner::update(float dt) {
    for (auto& agent : agents_) {
        if (agent.planned_path.empty() || agent.path_index >= agent.planned_path.size()) {
            continue;
        }
        
        // Move toward next waypoint
        Vec2 target = agent.planned_path[agent.path_index];
        Vec2 direction = target - agent.position;
        float dist = direction.length();
        
        if (dist < 0.5f) {
            agent.path_index++;
            if (agent.path_index >= agent.planned_path.size()) {
                agent.velocity = Vec2(0, 0);
                continue;
            }
            target = agent.planned_path[agent.path_index];
            direction = target - agent.position;
            dist = direction.length();
        }
        
        if (dist > 0.01f) {
            agent.velocity = direction * (1.0f / dist);
            
            // Apply collision avoidance
            Vec2 avoid_vel = calculateAvoidanceVelocity(static_cast<int>(&agent - &agents_[0]));
            agent.velocity = agent.velocity * 0.7f + avoid_vel * 0.3f;
            
            // Update position
            agent.position = agent.position + agent.velocity * dt;
        }
    }
}
