// demo.cpp - Simple A* demonstration without GUI
// Compile: See QUICKSTART.md for instructions

#include <iostream>
#include <iomanip>
#include "core/grid.h"
#include "core/astar.h"

void printGrid(const Grid& grid, const AStarResult& result, Vec2i start, Vec2i goal) {
    std::cout << "\nGrid Visualization:\n";
    std::cout << "  # = obstacle, S = start, G = goal, * = path, . = visited, o = free\n\n";
    
    for (int y = 0; y < grid.getHeight(); y++) {
        std::cout << "  ";
        for (int x = 0; x < grid.getWidth(); x++) {
            Vec2i pos(x, y);
            
            if (pos == start) {
                std::cout << "S ";
            } else if (pos == goal) {
                std::cout << "G ";
            } else if (grid.isObstacle(x, y)) {
                std::cout << "# ";
            } else {
                // Check if in path
                bool in_path = false;
                for (const auto& p : result.path) {
                    if (p == pos) {
                        in_path = true;
                        break;
                    }
                }
                
                if (in_path) {
                    std::cout << "* ";
                } else {
                    // Check if visited
                    bool visited = false;
                    for (const auto& p : result.visited) {
                        if (p == pos) {
                            visited = true;
                            break;
                        }
                    }
                    std::cout << (visited ? ". " : "o ");
                }
            }
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

void runDemo1() {
    std::cout << "========================================\n";
    std::cout << "Demo 1: Simple path in empty 10x10 grid\n";
    std::cout << "========================================\n";
    
    Grid grid(10, 10);
    AStar planner(grid);
    
    Vec2i start(0, 0);
    Vec2i goal(9, 9);
    
    auto result = planner.findPath(start, goal);
    
    if (result.success) {
        std::cout << "✓ Path found!\n";
        std::cout << "  Path length: " << result.path.size() << " cells\n";
        std::cout << "  Path cost: " << std::fixed << std::setprecision(2) 
                  << result.path_cost << "\n";
        std::cout << "  Nodes expanded: " << result.nodes_expanded << "\n";
        
        printGrid(grid, result, start, goal);
    } else {
        std::cout << "✗ No path found\n";
    }
}

void runDemo2() {
    std::cout << "========================================\n";
    std::cout << "Demo 2: Path around vertical obstacle\n";
    std::cout << "========================================\n";
    
    Grid grid(10, 10);
    
    // Create vertical wall in middle (with gap)
    for (int y = 2; y < 8; y++) {
        grid.setObstacle(5, y, true);
    }
    
    AStar planner(grid);
    Vec2i start(2, 5);
    Vec2i goal(8, 5);
    
    auto result = planner.findPath(start, goal);
    
    if (result.success) {
        std::cout << "✓ Path found (goes around obstacle)!\n";
        std::cout << "  Path length: " << result.path.size() << " cells\n";
        std::cout << "  Path cost: " << std::fixed << std::setprecision(2) 
                  << result.path_cost << "\n";
        std::cout << "  Nodes expanded: " << result.nodes_expanded << "\n";
        
        printGrid(grid, result, start, goal);
    } else {
        std::cout << "✗ No path found\n";
    }
}

void runDemo3() {
    std::cout << "========================================\n";
    std::cout << "Demo 3: No path (completely blocked)\n";
    std::cout << "========================================\n";
    
    Grid grid(10, 10);
    
    // Create complete wall (no gap)
    for (int y = 0; y < 10; y++) {
        grid.setObstacle(5, y, true);
    }
    
    AStar planner(grid);
    Vec2i start(2, 5);
    Vec2i goal(8, 5);
    
    auto result = planner.findPath(start, goal);
    
    if (result.success) {
        std::cout << "✓ Path found!\n";
    } else {
        std::cout << "✗ No path found (as expected - wall blocks all paths)\n";
        std::cout << "  Nodes expanded: " << result.nodes_expanded 
                  << " (explored before giving up)\n";
        
        printGrid(grid, result, start, goal);
    }
}

void runDemo4() {
    std::cout << "========================================\n";
    std::cout << "Demo 4: Maze-like environment\n";
    std::cout << "========================================\n";
    
    Grid grid(15, 15);
    
    // Create maze-like obstacles
    for (int i = 2; i < 13; i++) {
        if (i != 7) {  // Leave gaps
            grid.setObstacle(5, i, true);
            grid.setObstacle(10, i, true);
        }
    }
    
    AStar planner(grid);
    Vec2i start(1, 7);
    Vec2i goal(13, 7);
    
    auto result = planner.findPath(start, goal);
    
    if (result.success) {
        std::cout << "✓ Path found through maze!\n";
        std::cout << "  Path length: " << result.path.size() << " cells\n";
        std::cout << "  Path cost: " << std::fixed << std::setprecision(2) 
                  << result.path_cost << "\n";
        std::cout << "  Nodes expanded: " << result.nodes_expanded << "\n";
        
        printGrid(grid, result, start, goal);
    } else {
        std::cout << "✗ No path found\n";
    }
}

int main() {
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════╗\n";
    std::cout << "║  Autonomous Path Planner - A* Demonstration  ║\n";
    std::cout << "║          Tesla Project - Core Demo           ║\n";
    std::cout << "╚═══════════════════════════════════════════════╝\n";
    std::cout << "\n";
    
    runDemo1();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    runDemo2();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    runDemo3();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    runDemo4();
    
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "Demo complete! A* algorithm working.\n";
    std::cout << "========================================\n";
    std::cout << "\nNext: Install SDL2 for interactive GUI\n";
    std::cout << "See INSTALL.md for instructions.\n\n";
    
    return 0;
}
