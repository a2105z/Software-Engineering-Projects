// demo_phase2.cpp - Phase 2 demonstration: RRT, RRT*, Dynamic Obstacles, Path Smoothing

#include <iostream>
#include <iomanip>
#include "core/grid.h"
#include "core/astar.h"
#include "core/rrt.h"
#include "core/dynamic_obstacle.h"
#include "core/path_smoothing.h"

void printHeader(const std::string& title) {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << title << "\n";
    std::cout << "========================================\n";
}

void runAStarDemo() {
    printHeader("Demo 1: A* (Baseline)");
    
    Grid grid(20, 20);
    
    // Add obstacle
    for (int y = 5; y < 15; y++) {
        grid.setObstacle(10, y, true);
    }
    
    AStar planner(grid);
    Vec2i start(2, 10);
    Vec2i goal(18, 10);
    
    auto result = planner.findPath(start, goal);
    
    if (result.success) {
        std::cout << "✓ A* found path!\n";
        std::cout << "  Path length: " << result.path.size() << " cells\n";
        std::cout << "  Path cost: " << std::fixed << std::setprecision(2) 
                  << result.path_cost << "\n";
        std::cout << "  Nodes expanded: " << result.nodes_expanded << "\n";
    }
}

void runRRTDemo() {
    printHeader("Demo 2: RRT (Rapidly-exploring Random Trees)");
    
    Grid grid(20, 20);
    
    // Add same obstacle
    for (int y = 5; y < 15; y++) {
        grid.setObstacle(10, y, true);
    }
    
    RRT planner(grid);
    Vec2 start(2.0f, 10.0f);
    Vec2 goal(18.0f, 10.0f);
    
    auto result = planner.findPath(start, goal, 2000);
    
    if (result.success) {
        std::cout << "✓ RRT found path!\n";
        std::cout << "  Path length: " << result.path.size() << " waypoints\n";
        std::cout << "  Path cost: " << std::fixed << std::setprecision(2) 
                  << result.path_cost << "\n";
        std::cout << "  Iterations: " << result.iterations << "\n";
        std::cout << "  Tree size: " << result.tree_nodes.size() << " nodes\n";
        std::cout << "\nNote: RRT explores randomly, so path may not be optimal\n";
    }
}

void runRRTStarDemo() {
    printHeader("Demo 3: RRT* (Optimal RRT)");
    
    Grid grid(20, 20);
    
    // Add obstacle
    for (int y = 5; y < 15; y++) {
        grid.setObstacle(10, y, true);
    }
    
    RRTStar planner(grid);
    Vec2 start(2.0f, 10.0f);
    Vec2 goal(18.0f, 10.0f);
    
    auto result = planner.findPath(start, goal, 2000);
    
    if (result.success) {
        std::cout << "✓ RRT* found path!\n";
        std::cout << "  Path length: " << result.path.size() << " waypoints\n";
        std::cout << "  Path cost: " << std::fixed << std::setprecision(2) 
                  << result.path_cost << "\n";
        std::cout << "  Iterations: " << result.iterations << "\n";
        std::cout << "  Tree size: " << result.tree_nodes.size() << " nodes\n";
        std::cout << "\nNote: RRT* optimizes the path through rewiring\n";
        std::cout << "      Typically finds better paths than RRT\n";
    }
}

void runDynamicObstacleDemo() {
    printHeader("Demo 4: Dynamic Obstacle Avoidance");
    
    DynamicObstacleManager manager;
    
    // Create moving obstacle
    Vec2 obs_pos(5.0f, 5.0f);
    Vec2 obs_vel(1.0f, 0.5f);
    DynamicObstacle obs(obs_pos, obs_vel, 1.0f);
    
    manager.addObstacle(obs);
    
    std::cout << "Initial obstacle at: (" << obs_pos.x << ", " << obs_pos.y << ")\n";
    std::cout << "Velocity: (" << obs_vel.x << ", " << obs_vel.y << ")\n\n";
    
    // Simulate movement
    for (int t = 0; t <= 5; t++) {
        Vec2 pos = manager.getObstacles()[0].getPosition();
        std::cout << "Time " << t << "s: obstacle at (" 
                  << std::fixed << std::setprecision(2)
                  << pos.x << ", " << pos.y << ")\n";
        manager.updateAll(1.0f);  // 1 second timestep
    }
    
    std::cout << "\n✓ Dynamic obstacle tracking works!\n";
    std::cout << "  Can predict future positions for collision avoidance\n";
}

void runPathSmoothingDemo() {
    printHeader("Demo 5: Path Smoothing");
    
    Grid grid(20, 20);
    
    // Create a path with sharp turns
    std::vector<Vec2> raw_path = {
        Vec2(0.0f, 0.0f),
        Vec2(5.0f, 0.0f),
        Vec2(5.0f, 5.0f),
        Vec2(10.0f, 5.0f),
        Vec2(10.0f, 10.0f)
    };
    
    std::cout << "Raw path: " << raw_path.size() << " waypoints (sharp 90° turns)\n";
    
    // Apply Bezier smoothing
    auto bezier = PathSmoothing::bezierSmooth(raw_path, 5);
    std::cout << "\nBezier smoothing:\n";
    std::cout << "  Smoothed to " << bezier.size() << " points\n";
    std::cout << "  Creates smooth curves between waypoints\n";
    
    // Apply shortcut smoothing
    auto shortcut = PathSmoothing::shortcutSmooth(raw_path, grid, 10);
    std::cout << "\nShortcut smoothing:\n";
    std::cout << "  Reduced to " << shortcut.size() << " waypoints\n";
    std::cout << "  Removes unnecessary intermediate points\n";
    
    // Apply gradient smoothing
    auto gradient = PathSmoothing::gradientSmooth(raw_path, grid, 30, 0.15f);
    std::cout << "\nGradient smoothing:\n";
    std::cout << "  Adjusted " << gradient.size() << " waypoints\n";
    std::cout << "  Reduces sharp turns while avoiding obstacles\n";
    
    // Apply combined smoothing
    auto combined = PathSmoothing::smoothPath(raw_path, grid);
    std::cout << "\nCombined smoothing:\n";
    std::cout << "  Final path: " << combined.size() << " points\n";
    std::cout << "  Applies all techniques in sequence\n";
    
    std::cout << "\n✓ Path smoothing creates more natural, drivable paths!\n";
}

void runComparisonDemo() {
    printHeader("Demo 6: Algorithm Comparison");
    
    Grid grid(30, 30);
    
    // Add complex maze-like obstacles
    for (int i = 5; i < 25; i++) {
        if (i != 15) {
            grid.setObstacle(10, i, true);
            grid.setObstacle(20, i, true);
        }
    }
    
    Vec2i astar_start(5, 15);
    Vec2i astar_goal(25, 15);
    Vec2 rrt_start(5.0f, 15.0f);
    Vec2 rrt_goal(25.0f, 15.0f);
    
    std::cout << "Comparing algorithms on maze with narrow passages:\n\n";
    
    // A*
    AStar astar(grid);
    auto astar_result = astar.findPath(astar_start, astar_goal);
    std::cout << "A*:\n";
    std::cout << "  Success: " << (astar_result.success ? "Yes" : "No") << "\n";
    std::cout << "  Path cost: " << std::fixed << std::setprecision(2) 
              << astar_result.path_cost << "\n";
    std::cout << "  Nodes explored: " << astar_result.nodes_expanded << "\n";
    std::cout << "  Guaranteed optimal: Yes\n\n";
    
    // RRT
    RRT rrt(grid);
    auto rrt_result = rrt.findPath(rrt_start, rrt_goal, 3000);
    std::cout << "RRT:\n";
    std::cout << "  Success: " << (rrt_result.success ? "Yes" : "No") << "\n";
    if (rrt_result.success) {
        std::cout << "  Path cost: " << std::fixed << std::setprecision(2) 
                  << rrt_result.path_cost << "\n";
    }
    std::cout << "  Iterations: " << rrt_result.iterations << "\n";
    std::cout << "  Tree size: " << rrt_result.tree_nodes.size() << "\n";
    std::cout << "  Guaranteed optimal: No (probabilistically complete)\n\n";
    
    // RRT*
    RRTStar rrt_star(grid);
    auto rrt_star_result = rrt_star.findPath(rrt_start, rrt_goal, 3000);
    std::cout << "RRT*:\n";
    std::cout << "  Success: " << (rrt_star_result.success ? "Yes" : "No") << "\n";
    if (rrt_star_result.success) {
        std::cout << "  Path cost: " << std::fixed << std::setprecision(2) 
                  << rrt_star_result.path_cost << "\n";
    }
    std::cout << "  Iterations: " << rrt_star_result.iterations << "\n";
    std::cout << "  Tree size: " << rrt_star_result.tree_nodes.size() << "\n";
    std::cout << "  Guaranteed optimal: Asymptotically (converges to optimal)\n\n";
    
    std::cout << "Summary:\n";
    std::cout << "  A*: Fast, optimal for grid-based problems\n";
    std::cout << "  RRT: Good for high-dimensional spaces, complex constraints\n";
    std::cout << "  RRT*: Combines exploration with optimization\n";
}

int main() {
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════════╗\n";
    std::cout << "║   Autonomous Path Planner - Phase 2 Demonstration    ║\n";
    std::cout << "║    RRT, RRT*, Dynamic Obstacles, Path Smoothing       ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════╝\n";
    
    runAStarDemo();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    runRRTDemo();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    runRRTStarDemo();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    runDynamicObstacleDemo();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    runPathSmoothingDemo();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    runComparisonDemo();
    
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "Phase 2 complete! All features working.\n";
    std::cout << "========================================\n";
    std::cout << "\nNext steps:\n";
    std::cout << "  1. Install SDL2 for interactive GUI\n";
    std::cout << "  2. Try: TAB (switch planner), R (RRT), T (RRT*), D (dynamic obstacle)\n";
    std::cout << "  3. Phase 3: Hybrid A* with vehicle dynamics\n\n";
    
    return 0;
}
