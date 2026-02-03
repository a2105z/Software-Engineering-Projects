#include "gui/app.h"
#include "core/path_smoothing.h"
#include <iostream>
#include <random>

App::App(int window_width, int window_height, int grid_width, int grid_height) 
    : is_dragging_(false)
    , is_erasing_(false)
    , current_planner_(PlannerType::ASTAR)
    , show_smoothed_(false)
    , paused_(false)
    , simulation_time_(0.0f) {
    
    grid_ = std::make_unique<Grid>(grid_width, grid_height);
    
    int cell_size = std::min(window_width / grid_width, window_height / grid_height);
    renderer_ = std::make_unique<Renderer>(window_width, window_height, cell_size);
    
    astar_planner_ = std::make_unique<AStar>(*grid_);
    rrt_planner_ = std::make_unique<RRT>(*grid_);
    rrt_star_planner_ = std::make_unique<RRTStar>(*grid_);
    dynamic_obstacles_ = std::make_unique<DynamicObstacleManager>();
    
    std::cout << "===================================================\n";
    std::cout << " Autonomous Path Planner - Phase 2 Features\n";
    std::cout << "===================================================\n";
    std::cout << "Mouse Controls:\n";
    std::cout << "  Left Click + Drag: Draw obstacles\n";
    std::cout << "  Right Click: Set start position (green)\n";
    std::cout << "  Shift + Left Click: Set goal position (red)\n";
    std::cout << "\nPlanning:\n";
    std::cout << "  A: Run A* pathfinding\n";
    std::cout << "  R: Run RRT pathfinding\n";
    std::cout << "  T: Run RRT* pathfinding\n";
    std::cout << "  S: Toggle path smoothing\n";
    std::cout << "  TAB: Switch planner\n";
    std::cout << "\nDynamic Obstacles:\n";
    std::cout << "  D: Add random dynamic obstacle\n";
    std::cout << "  SPACE: Pause/Resume simulation\n";
    std::cout << "\nClear:\n";
    std::cout << "  C: Clear everything\n";
    std::cout << "  O: Clear obstacles only\n";
    std::cout << "  P: Clear path only\n";
    std::cout << "  ESC/Q: Quit\n";
    std::cout << "===================================================\n\n";
}

App::~App() = default;

void App::run() {
    bool running = true;
    Uint32 last_time = SDL_GetTicks();
    
    while (running) {
        Uint32 current_time = SDL_GetTicks();
        float dt = (current_time - last_time) / 1000.0f;
        last_time = current_time;
        
        handleEvents(running);
        
        if (!paused_) {
            updateSimulation(dt);
        }
        
        render();
        SDL_Delay(16);  // ~60 FPS
    }
}

void App::updateSimulation(float dt) {
    simulation_time_ += dt;
    dynamic_obstacles_->updateAll(dt);
}

void App::handleEvents(bool& running) {
    SDL_Event event;
    
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
                
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                handleMouseButton(event.button);
                break;
                
            case SDL_MOUSEMOTION:
                handleMouseMotion(event.motion);
                break;
                
            case SDL_KEYDOWN:
                handleKeyPress(event.key);
                if (event.key.keysym.sym == SDLK_ESCAPE || 
                    event.key.keysym.sym == SDLK_q) {
                    running = false;
                }
                break;
        }
    }
}

void App::handleMouseButton(const SDL_MouseButtonEvent& event) {
    Vec2i grid_pos = renderer_->screenToGrid(event.x, event.y);
    
    if (!grid_->isValid(grid_pos.x, grid_pos.y)) {
        return;
    }
    
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button == SDL_BUTTON_LEFT) {
            SDL_Keymod mod = SDL_GetModState();
            if (mod & KMOD_SHIFT) {
                goal_ = grid_pos;
                std::cout << "Goal set at (" << grid_pos.x << ", " << grid_pos.y << ")\n";
            } else {
                is_dragging_ = true;
                is_erasing_ = grid_->isObstacle(grid_pos.x, grid_pos.y);
                grid_->setObstacle(grid_pos.x, grid_pos.y, !is_erasing_);
            }
        } else if (event.button == SDL_BUTTON_RIGHT) {
            start_ = grid_pos;
            std::cout << "Start set at (" << grid_pos.x << ", " << grid_pos.y << ")\n";
        }
    } else if (event.type == SDL_MOUSEBUTTONUP) {
        if (event.button == SDL_BUTTON_LEFT) {
            is_dragging_ = false;
        }
    }
}

void App::handleMouseMotion(const SDL_MouseMotionEvent& event) {
    if (is_dragging_) {
        Vec2i grid_pos = renderer_->screenToGrid(event.x, event.y);
        if (grid_->isValid(grid_pos.x, grid_pos.y)) {
            grid_->setObstacle(grid_pos.x, grid_pos.y, !is_erasing_);
        }
    }
}

void App::handleKeyPress(const SDL_KeyboardEvent& event) {
    switch (event.keysym.sym) {
        case SDLK_a:
            current_planner_ = PlannerType::ASTAR;
            runPathfinding();
            break;
            
        case SDLK_r:
            current_planner_ = PlannerType::RRT;
            runPathfinding();
            break;
            
        case SDLK_t:
            current_planner_ = PlannerType::RRT_STAR;
            runPathfinding();
            break;
            
        case SDLK_TAB:
            switchPlanner();
            break;
            
        case SDLK_s:
            toggleSmoothing();
            break;
            
        case SDLK_d:
            addRandomDynamicObstacle();
            break;
            
        case SDLK_SPACE:
            togglePause();
            break;
            
        case SDLK_c:
            clearAll();
            break;
            
        case SDLK_o:
            clearObstacles();
            break;
            
        case SDLK_p:
            clearPath();
            break;
    }
}

void App::runPathfinding() {
    if (!start_.has_value() || !goal_.has_value()) {
        std::cout << "Please set both start (right-click) and goal (shift+left-click) first!\n";
        return;
    }
    
    Vec2 start_f(static_cast<float>(start_->x), static_cast<float>(start_->y));
    Vec2 goal_f(static_cast<float>(goal_->x), static_cast<float>(goal_->y));
    
    switch (current_planner_) {
        case PlannerType::ASTAR: {
            std::cout << "Running A* from (" << start_->x << ", " << start_->y 
                      << ") to (" << goal_->x << ", " << goal_->y << ")...\n";
            astar_result_ = astar_planner_->findPath(start_.value(), goal_.value());
            
            if (astar_result_.success) {
                std::cout << "A* Path found!\n";
                std::cout << "  Path length: " << astar_result_.path.size() << " cells\n";
                std::cout << "  Path cost: " << astar_result_.path_cost << "\n";
                std::cout << "  Nodes expanded: " << astar_result_.nodes_expanded << "\n";
            } else {
                std::cout << "No path found.\n";
            }
            break;
        }
        
        case PlannerType::RRT: {
            std::cout << "Running RRT...\n";
            rrt_result_ = rrt_planner_->findPath(start_f, goal_f, 3000);
            
            if (rrt_result_.success) {
                std::cout << "RRT Path found!\n";
                std::cout << "  Path length: " << rrt_result_.path.size() << " waypoints\n";
                std::cout << "  Path cost: " << rrt_result_.path_cost << "\n";
                std::cout << "  Iterations: " << rrt_result_.iterations << "\n";
                std::cout << "  Tree size: " << rrt_result_.tree_nodes.size() << " nodes\n";
            } else {
                std::cout << "RRT did not reach goal (partial path shown).\n";
                std::cout << "  Iterations: " << rrt_result_.iterations << "\n";
            }
            break;
        }
        
        case PlannerType::RRT_STAR: {
            std::cout << "Running RRT*...\n";
            rrt_result_ = rrt_star_planner_->findPath(start_f, goal_f, 3000);
            
            if (rrt_result_.success) {
                std::cout << "RRT* Path found!\n";
                std::cout << "  Path length: " << rrt_result_.path.size() << " waypoints\n";
                std::cout << "  Path cost: " << rrt_result_.path_cost << "\n";
                std::cout << "  Iterations: " << rrt_result_.iterations << "\n";
                std::cout << "  Tree size: " << rrt_result_.tree_nodes.size() << " nodes\n";
            } else {
                std::cout << "RRT* did not reach goal (partial path shown).\n";
            }
            break;
        }
    }
}

void App::switchPlanner() {
    switch (current_planner_) {
        case PlannerType::ASTAR:
            current_planner_ = PlannerType::RRT;
            std::cout << "Switched to RRT planner\n";
            break;
        case PlannerType::RRT:
            current_planner_ = PlannerType::RRT_STAR;
            std::cout << "Switched to RRT* planner\n";
            break;
        case PlannerType::RRT_STAR:
            current_planner_ = PlannerType::ASTAR;
            std::cout << "Switched to A* planner\n";
            break;
    }
}

void App::toggleSmoothing() {
    show_smoothed_ = !show_smoothed_;
    std::cout << "Path smoothing: " << (show_smoothed_ ? "ON" : "OFF") << "\n";
}

void App::togglePause() {
    paused_ = !paused_;
    std::cout << "Simulation: " << (paused_ ? "PAUSED" : "RUNNING") << "\n";
}

void App::addRandomDynamicObstacle() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist_x(5.0f, grid_->getWidth() - 5.0f);
    std::uniform_real_distribution<float> dist_y(5.0f, grid_->getHeight() - 5.0f);
    std::uniform_real_distribution<float> dist_v(-2.0f, 2.0f);
    
    Vec2 pos(dist_x(gen), dist_y(gen));
    Vec2 vel(dist_v(gen), dist_v(gen));
    
    DynamicObstacle obs(pos, vel, 1.0f);
    dynamic_obstacles_->addObstacle(obs);
    
    std::cout << "Added dynamic obstacle at (" << pos.x << ", " << pos.y 
              << ") with velocity (" << vel.x << ", " << vel.y << ")\n";
    std::cout << "Total dynamic obstacles: " << dynamic_obstacles_->size() << "\n";
}

void App::clearAll() {
    grid_->clear();
    start_.reset();
    goal_.reset();
    astar_result_ = AStarResult();
    rrt_result_ = RRTResult();
    dynamic_obstacles_->clear();
    simulation_time_ = 0.0f;
    std::cout << "Cleared everything.\n";
}

void App::clearObstacles() {
    grid_->clear();
    std::cout << "Cleared obstacles.\n";
}

void App::clearPath() {
    astar_result_ = AStarResult();
    rrt_result_ = RRTResult();
    std::cout << "Cleared path.\n";
}

void App::render() {
    renderer_->clear();
    renderer_->drawGrid(*grid_);
    
    // Draw based on current planner
    if (current_planner_ == PlannerType::ASTAR) {
        // Draw A* search visualization
        if (!astar_result_.visited.empty()) {
            renderer_->drawSearchProgress(astar_result_.visited, astar_result_.explored);
        }
        
        // Draw A* path
        if (!astar_result_.path.empty()) {
            if (show_smoothed_) {
                // Convert to Vec2 and smooth
                std::vector<Vec2> path_f;
                for (const auto& p : astar_result_.path) {
                    path_f.push_back(Vec2(static_cast<float>(p.x), static_cast<float>(p.y)));
                }
                auto smoothed = PathSmoothing::smoothPath(path_f, *grid_);
                renderer_->drawSmoothPath(smoothed);
            } else {
                renderer_->drawPath(astar_result_.path);
            }
        }
    } else {
        // Draw RRT/RRT* tree and path
        if (!rrt_result_.tree_nodes.empty()) {
            renderer_->drawRRTTree(rrt_result_.tree_nodes);
        }
        
        if (!rrt_result_.path.empty()) {
            if (show_smoothed_) {
                auto smoothed = PathSmoothing::smoothPath(rrt_result_.path, *grid_);
                renderer_->drawSmoothPath(smoothed);
            } else {
                renderer_->drawSmoothPath(rrt_result_.path);
            }
        }
    }
    
    // Draw dynamic obstacles
    if (dynamic_obstacles_->size() > 0) {
        renderer_->drawDynamicObstacles(*dynamic_obstacles_);
    }
    
    // Draw start and goal markers
    if (start_.has_value()) {
        renderer_->drawStart(start_.value());
    }
    if (goal_.has_value()) {
        renderer_->drawGoal(goal_.value());
    }
    
    // Draw status text
    std::string planner_name;
    switch (current_planner_) {
        case PlannerType::ASTAR: planner_name = "A*"; break;
        case PlannerType::RRT: planner_name = "RRT"; break;
        case PlannerType::RRT_STAR: planner_name = "RRT*"; break;
    }
    
    std::string status = "Planner: " + planner_name + 
                        " | Smooth: " + (show_smoothed_ ? "ON" : "OFF") +
                        " | Dyn.Obs: " + std::to_string(dynamic_obstacles_->size()) +
                        " | " + (paused_ ? "PAUSED" : "RUNNING");
    renderer_->drawText(status, 10, 10);
    
    renderer_->present();
}
