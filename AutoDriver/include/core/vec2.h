#pragma once

#include <cmath>

/**
 * Simple 2D vector for positions and directions.
 */
struct Vec2 {
    float x, y;
    
    Vec2(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
    
    Vec2 operator+(const Vec2& other) const {
        return Vec2(x + other.x, y + other.y);
    }
    
    Vec2 operator-(const Vec2& other) const {
        return Vec2(x - other.x, y - other.y);
    }
    
    Vec2 operator*(float scalar) const {
        return Vec2(x * scalar, y * scalar);
    }
    
    float length() const {
        return std::sqrt(x * x + y * y);
    }
    
    float distanceTo(const Vec2& other) const {
        return (*this - other).length();
    }
};

/**
 * Integer 2D coordinate for grid cells.
 */
struct Vec2i {
    int x, y;
    
    Vec2i(int x = 0, int y = 0) : x(x), y(y) {}
    
    bool operator==(const Vec2i& other) const {
        return x == other.x && y == other.y;
    }
    
    bool operator!=(const Vec2i& other) const {
        return !(*this == other);
    }
};
