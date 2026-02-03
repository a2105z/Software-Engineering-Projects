#pragma once

#include <memory>
#include <optional>
#include "renderer.h"
#include "core/grid.h"
#include "core/astar.h"
#include "core/rrt.h"
#include "core/dynamic_obstacle.h"
#include "core/vec2.h"

/**
 * Planner type selection.
 */
enum class PlannerType {
    ASTAR,
    RRT,
    RRT_STAR
};

/**
 * Main application class handling the interactive GUI.
 */
class App {
public:
    App(int window_width, int window_height, int grid_width, int grid_height);
    ~App();
    
    // Main loop
    void run();
    
private:
    // Core components
    std::unique_ptr<Grid> grid_;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<AStar> astar_planner_;
    std::unique_ptr<RRT> rrt_planner_;
    std::unique_ptr<RRTStar> rrt_star_planner_;
    std::unique_ptr<DynamicObstacleManager> dynamic_obstacles_;
    
    // State
    std::optional<Vec2i> start_;
    std::optional<Vec2i> goal_;
    AStarResult astar_result_;
    RRTResult rrt_result_;
    PlannerType current_planner_;
    bool is_dragging_;
    bool is_erasing_;
    bool show_smoothed_;
    bool paused_;
    float simulation_time_;
    
    // Event handling
    void handleEvents(bool& running);
    void handleMouseButton(const SDL_MouseButtonEvent& event);
    void handleMouseMotion(const SDL_MouseMotionEvent& event);
    void handleKeyPress(const SDL_KeyboardEvent& event);
    
    // Actions
    void runPathfinding();
    void clearAll();
    void clearObstacles();
    void clearPath();
    void switchPlanner();
    void toggleSmoothing();
    void togglePause();
    void addRandomDynamicObstacle();
    
    // Rendering
    void render();
    void updateSimulation(float dt);
};
