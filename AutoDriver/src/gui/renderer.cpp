#include "gui/renderer.h"
#include <iostream>

Renderer::Renderer(int window_width, int window_height, int cell_size)
    : cell_size_(cell_size)
    , window_width_(window_width)
    , window_height_(window_height) {
    
    window_ = SDL_CreateWindow(
        "AutoDriver - Autonomous Path Planner",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
        SDL_WINDOW_SHOWN
    );
    
    if (!window_) {
        std::cerr << "Failed to create window: " << SDL_GetError() << "\n";
        return;
    }
    
    renderer_ = SDL_CreateRenderer(
        window_,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    
    if (!renderer_) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << "\n";
    }
}

Renderer::~Renderer() {
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_) SDL_DestroyWindow(window_);
}

void Renderer::clear() {
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);  // White background
    SDL_RenderClear(renderer_);
}

void Renderer::drawGrid(const Grid& grid) {
    // Draw grid lines
    SDL_SetRenderDrawColor(renderer_, 220, 220, 220, 255);  // Light gray
    
    for (int x = 0; x <= grid.getWidth(); x++) {
        SDL_RenderDrawLine(renderer_, 
            x * cell_size_, 0,
            x * cell_size_, grid.getHeight() * cell_size_
        );
    }
    
    for (int y = 0; y <= grid.getHeight(); y++) {
        SDL_RenderDrawLine(renderer_,
            0, y * cell_size_,
            grid.getWidth() * cell_size_, y * cell_size_
        );
    }
    
    // Draw obstacles
    for (int y = 0; y < grid.getHeight(); y++) {
        for (int x = 0; x < grid.getWidth(); x++) {
            if (grid.isObstacle(x, y)) {
                fillCell(x, y, 40, 40, 40);  // Dark gray/black
            }
        }
    }
}

void Renderer::drawSearchProgress(const std::vector<Vec2i>& visited, 
                                  const std::vector<Vec2i>& exploring) {
    // Draw visited nodes (closed set) - light blue
    for (const Vec2i& pos : visited) {
        fillCell(pos.x, pos.y, 200, 220, 255, 180);  // Light blue, semi-transparent
    }
    
    // Draw exploring nodes (open set) - yellow
    for (const Vec2i& pos : exploring) {
        fillCell(pos.x, pos.y, 255, 255, 150, 200);  // Yellow, semi-transparent
    }
}

void Renderer::drawPath(const std::vector<Vec2i>& path) {
    if (path.empty()) return;
    
    // Draw path as connected blue line
    SDL_SetRenderDrawColor(renderer_, 50, 100, 255, 255);  // Blue
    
    for (size_t i = 0; i < path.size(); i++) {
        // Fill each cell on the path
        fillCell(path[i].x, path[i].y, 100, 150, 255, 200);
        
        // Draw line to next cell
        if (i < path.size() - 1) {
            int x1 = path[i].x * cell_size_ + cell_size_ / 2;
            int y1 = path[i].y * cell_size_ + cell_size_ / 2;
            int x2 = path[i + 1].x * cell_size_ + cell_size_ / 2;
            int y2 = path[i + 1].y * cell_size_ + cell_size_ / 2;
            
            // Draw thicker line
            for (int offset = -2; offset <= 2; offset++) {
                SDL_RenderDrawLine(renderer_, x1 + offset, y1, x2 + offset, y2);
                SDL_RenderDrawLine(renderer_, x1, y1 + offset, x2, y2 + offset);
            }
        }
    }
}

void Renderer::drawStart(Vec2i pos) {
    // Draw start as green circle
    fillCell(pos.x, pos.y, 100, 255, 100);  // Bright green
    
    // Draw "S" or circle marker
    int cx = pos.x * cell_size_ + cell_size_ / 2;
    int cy = pos.y * cell_size_ + cell_size_ / 2;
    int radius = cell_size_ / 3;
    
    SDL_SetRenderDrawColor(renderer_, 0, 150, 0, 255);  // Dark green
    // Simple circle approximation
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx*dx + dy*dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer_, cx + dx, cy + dy);
            }
        }
    }
}

void Renderer::drawGoal(Vec2i pos) {
    // Draw goal as red square
    fillCell(pos.x, pos.y, 255, 100, 100);  // Bright red
    
    // Draw inner square marker
    int x = pos.x * cell_size_ + cell_size_ / 4;
    int y = pos.y * cell_size_ + cell_size_ / 4;
    int size = cell_size_ / 2;
    
    SDL_SetRenderDrawColor(renderer_, 200, 0, 0, 255);  // Dark red
    SDL_Rect rect{x, y, size, size};
    SDL_RenderFillRect(renderer_, &rect);
}

void Renderer::present() {
    SDL_RenderPresent(renderer_);
}

Vec2i Renderer::screenToGrid(int screen_x, int screen_y) const {
    return Vec2i(screen_x / cell_size_, screen_y / cell_size_);
}

void Renderer::fillCell(int grid_x, int grid_y, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(renderer_, r, g, b, a);
    SDL_Rect rect{
        grid_x * cell_size_,
        grid_y * cell_size_,
        cell_size_,
        cell_size_
    };
    SDL_RenderFillRect(renderer_, &rect);
}

void Renderer::drawCircle(int cx, int cy, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx*dx + dy*dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer_, cx + dx, cy + dy);
            }
        }
    }
}

void Renderer::drawLine(Vec2 from, Vec2 to, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(renderer_, r, g, b, a);
    int x1 = static_cast<int>(from.x * cell_size_);
    int y1 = static_cast<int>(from.y * cell_size_);
    int x2 = static_cast<int>(to.x * cell_size_);
    int y2 = static_cast<int>(to.y * cell_size_);
    SDL_RenderDrawLine(renderer_, x1, y1, x2, y2);
}

void Renderer::drawSmoothPath(const std::vector<Vec2>& path) {
    if (path.size() < 2) return;
    
    SDL_SetRenderDrawColor(renderer_, 50, 150, 255, 255);  // Blue
    
    for (size_t i = 0; i < path.size() - 1; i++) {
        int x1 = static_cast<int>(path[i].x * cell_size_);
        int y1 = static_cast<int>(path[i].y * cell_size_);
        int x2 = static_cast<int>(path[i + 1].x * cell_size_);
        int y2 = static_cast<int>(path[i + 1].y * cell_size_);
        
        // Draw thicker line
        for (int offset = -2; offset <= 2; offset++) {
            SDL_RenderDrawLine(renderer_, x1 + offset, y1, x2 + offset, y2);
            SDL_RenderDrawLine(renderer_, x1, y1 + offset, x2, y2 + offset);
        }
    }
}

void Renderer::drawRRTTree(const std::vector<Vec2>& tree_nodes) {
    // Draw tree nodes as small points
    SDL_SetRenderDrawColor(renderer_, 150, 150, 150, 100);  // Light gray, transparent
    
    for (const auto& node : tree_nodes) {
        int x = static_cast<int>(node.x * cell_size_);
        int y = static_cast<int>(node.y * cell_size_);
        
        // Draw small circle for each node
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                SDL_RenderDrawPoint(renderer_, x + dx, y + dy);
            }
        }
    }
}

void Renderer::drawDynamicObstacles(const DynamicObstacleManager& obstacles) {
    SDL_SetRenderDrawColor(renderer_, 255, 100, 100, 200);  // Red, semi-transparent
    
    for (const auto& obs : obstacles.getObstacles()) {
        Vec2 pos = obs.getPosition();
        float radius = obs.getRadius();
        
        int cx = static_cast<int>(pos.x * cell_size_);
        int cy = static_cast<int>(pos.y * cell_size_);
        int r = static_cast<int>(radius * cell_size_);
        
        drawCircle(cx, cy, r);
        
        // Draw velocity vector
        Vec2 vel = obs.getVelocity();
        if (vel.length() > 0.01f) {
            SDL_SetRenderDrawColor(renderer_, 255, 200, 0, 255);  // Orange
            int vx = static_cast<int>((pos.x + vel.x * 2.0f) * cell_size_);
            int vy = static_cast<int>((pos.y + vel.y * 2.0f) * cell_size_);
            SDL_RenderDrawLine(renderer_, cx, cy, vx, vy);
        }
    }
}

void Renderer::drawText(const std::string& text, int x, int y) {
    // Simple text rendering placeholder
    // In a full implementation, you would use SDL_ttf for proper text rendering
    // For now, we just ensure the window title shows the mode
    (void)text;
    (void)x;
    (void)y;
}
