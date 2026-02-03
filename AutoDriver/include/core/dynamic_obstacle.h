#pragma once

#include <vector>
#include "vec2.h"

/**
 * Dynamic (moving) obstacle with velocity and trajectory prediction.
 */
class DynamicObstacle {
public:
    DynamicObstacle(Vec2 position, Vec2 velocity, float radius = 0.5f);
    
    // Update position based on velocity
    void update(float dt);
    
    // Predict position at future time
    Vec2 predictPosition(float time) const;
    
    // Check if point collides with obstacle at given time
    bool collides(Vec2 point, float time) const;
    
    // Check if line segment collides with obstacle trajectory
    bool collidesWithPath(Vec2 from, Vec2 to, float start_time, float end_time) const;
    
    // Getters
    Vec2 getPosition() const { return position_; }
    Vec2 getVelocity() const { return velocity_; }
    float getRadius() const { return radius_; }
    
    // Setters
    void setVelocity(Vec2 velocity) { velocity_ = velocity; }
    void setRadius(float radius) { radius_ = radius; }
    
private:
    Vec2 position_;
    Vec2 velocity_;
    float radius_;
};

/**
 * Manager for multiple dynamic obstacles.
 */
class DynamicObstacleManager {
public:
    DynamicObstacleManager() = default;
    
    // Add/remove obstacles
    void addObstacle(const DynamicObstacle& obstacle);
    void clear();
    
    // Update all obstacles
    void updateAll(float dt);
    
    // Collision checking
    bool checkCollision(Vec2 point, float time) const;
    bool checkPathCollision(Vec2 from, Vec2 to, float start_time, float end_time) const;
    
    // Getters
    const std::vector<DynamicObstacle>& getObstacles() const { return obstacles_; }
    size_t size() const { return obstacles_.size(); }
    
private:
    std::vector<DynamicObstacle> obstacles_;
};
