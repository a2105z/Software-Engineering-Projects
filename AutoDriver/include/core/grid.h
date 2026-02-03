#pragma once

#include <vector>
#include "vec2.h"

/**
 * 2D grid environment with obstacles.
 */
class Grid {
public:
    Grid(int width, int height);
    
    // Obstacle management
    bool isObstacle(int x, int y) const;
    void setObstacle(int x, int y, bool blocked);
    void toggleObstacle(int x, int y);
    void clear();
    
    // Bounds checking
    bool isValid(int x, int y) const;
    
    // Getters
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    
private:
    int width_, height_;
    std::vector<std::vector<bool>> obstacles_;
};
