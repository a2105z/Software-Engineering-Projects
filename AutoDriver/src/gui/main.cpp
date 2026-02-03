#include <SDL2/SDL.h>
#include <iostream>
#include "gui/app.h"

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << "\n";
        return 1;
    }
    
    std::cout << "=================================================\n";
    std::cout << " Autonomous Path Planner - Tesla Project\n";
    std::cout << " A* Pathfinding with Interactive Visualization\n";
    std::cout << "=================================================\n\n";
    
    try {
        // Create and run application
        // Window: 1000x1000, Grid: 50x50
        App app(1000, 1000, 50, 50);
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        SDL_Quit();
        return 1;
    }
    
    SDL_Quit();
    std::cout << "\nThanks for using the path planner!\n";
    return 0;
}
