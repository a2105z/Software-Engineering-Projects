#include "core/grid.h"

Grid::Grid(int width, int height) 
    : width_(width), height_(height) {
    obstacles_.resize(height, std::vector<bool>(width, false));
}

bool Grid::isObstacle(int x, int y) const {
    if (!isValid(x, y)) return true;  // Out of bounds = obstacle
    return obstacles_[y][x];
}

void Grid::setObstacle(int x, int y, bool blocked) {
    if (isValid(x, y)) {
        obstacles_[y][x] = blocked;
    }
}

void Grid::toggleObstacle(int x, int y) {
    if (isValid(x, y)) {
        obstacles_[y][x] = !obstacles_[y][x];
    }
}

void Grid::clear() {
    for (auto& row : obstacles_) {
        std::fill(row.begin(), row.end(), false);
    }
}

bool Grid::isValid(int x, int y) const {
    return x >= 0 && x < width_ && y >= 0 && y < height_;
}
