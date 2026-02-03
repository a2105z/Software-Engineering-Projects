#include "core/dynamic_obstacle.h"
#include <cmath>
#include <algorithm>

// ============================================================================
// DynamicObstacle Implementation
// ============================================================================

DynamicObstacle::DynamicObstacle(Vec2 position, Vec2 velocity, float radius)
    : position_(position), velocity_(velocity), radius_(radius) {
}

void DynamicObstacle::update(float dt) {
    position_ = position_ + velocity_ * dt;
}

Vec2 DynamicObstacle::predictPosition(float time) const {
    return position_ + velocity_ * time;
}

bool DynamicObstacle::collides(Vec2 point, float time) const {
    Vec2 future_pos = predictPosition(time);
    float dist = point.distanceTo(future_pos);
    return dist < radius_;
}

bool DynamicObstacle::collidesWithPath(Vec2 from, Vec2 to, 
                                       float start_time, float end_time) const {
    // Sample multiple time points along the path
    int num_samples = std::max(5, static_cast<int>((end_time - start_time) * 10.0f));
    
    for (int i = 0; i <= num_samples; i++) {
        float t_path = static_cast<float>(i) / num_samples;
        float time = start_time + (end_time - start_time) * t_path;
        
        Vec2 point = from + (to - from) * t_path;
        
        if (collides(point, time)) {
            return true;
        }
    }
    
    return false;
}

// ============================================================================
// DynamicObstacleManager Implementation
// ============================================================================

void DynamicObstacleManager::addObstacle(const DynamicObstacle& obstacle) {
    obstacles_.push_back(obstacle);
}

void DynamicObstacleManager::clear() {
    obstacles_.clear();
}

void DynamicObstacleManager::updateAll(float dt) {
    for (auto& obstacle : obstacles_) {
        obstacle.update(dt);
    }
}

bool DynamicObstacleManager::checkCollision(Vec2 point, float time) const {
    for (const auto& obstacle : obstacles_) {
        if (obstacle.collides(point, time)) {
            return true;
        }
    }
    return false;
}

bool DynamicObstacleManager::checkPathCollision(Vec2 from, Vec2 to, 
                                                float start_time, float end_time) const {
    for (const auto& obstacle : obstacles_) {
        if (obstacle.collidesWithPath(from, to, start_time, end_time)) {
            return true;
        }
    }
    return false;
}
