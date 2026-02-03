#pragma once

#include <SDL2/SDL.h>
#include <vector>
#include <optional>
#include "core/grid.h"
#include "core/vec2.h"
#include "core/astar.h"
#include "core/rrt.h"
#include "core/dynamic_obstacle.h"

/**
 * SDL2-based renderer for the path planning visualization.
 */
class Renderer {
public:
    Renderer(int window_width, int window_height, int cell_size);
    ~Renderer();
    
    // Core rendering
    void clear();
    void drawGrid(const Grid& grid);
    void drawPath(const std::vector<Vec2i>& path);
    void drawSmoothPath(const std::vector<Vec2>& path);
    void drawRRTTree(const std::vector<Vec2>& tree_nodes);
    void drawSearchProgress(const std::vector<Vec2i>& visited, 
                           const std::vector<Vec2i>& exploring);
    void drawDynamicObstacles(const DynamicObstacleManager& obstacles);
    void drawStart(Vec2i pos);
    void drawGoal(Vec2i pos);
    void drawText(const std::string& text, int x, int y);
    void present();
    
    // Coordinate conversion
    Vec2i screenToGrid(int screen_x, int screen_y) const;
    
    // Getters
    int getCellSize() const { return cell_size_; }
    
private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    int cell_size_;
    int window_width_;
    int window_height_;
    
    void fillCell(int grid_x, int grid_y, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    void drawCircle(int cx, int cy, int radius);
    void drawLine(Vec2 from, Vec2 to, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
};
