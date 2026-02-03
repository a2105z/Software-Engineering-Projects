#pragma once

#include <vector>
#include "vec2.h"
#include "grid.h"

/**
 * Path smoothing utilities for post-processing planned paths.
 */
class PathSmoothing {
public:
    /**
     * Smooth path using cubic Bezier curves.
     * Interpolates between waypoints using smooth curves.
     */
    static std::vector<Vec2> bezierSmooth(const std::vector<Vec2>& path,
                                          int points_per_segment = 10);
    
    /**
     * Shortcut smoothing: Remove unnecessary waypoints using line-of-sight.
     * Tries to connect non-adjacent waypoints directly if collision-free.
     */
    static std::vector<Vec2> shortcutSmooth(const std::vector<Vec2>& path,
                                           const Grid& grid,
                                           int max_iterations = 100);
    
    /**
     * Gradient descent smoothing: Minimize path curvature while staying collision-free.
     * Iteratively adjusts waypoints to reduce sharp turns.
     */
    static std::vector<Vec2> gradientSmooth(const std::vector<Vec2>& path,
                                           const Grid& grid,
                                           int iterations = 50,
                                           float alpha = 0.1f);
    
    /**
     * Combined smoothing: Apply multiple techniques in sequence.
     */
    static std::vector<Vec2> smoothPath(const std::vector<Vec2>& path,
                                       const Grid& grid);

private:
    // Helper: Check if line segment is collision-free
    static bool isLineCollisionFree(Vec2 from, Vec2 to, const Grid& grid);
    
    // Helper: Calculate cubic Bezier point
    static Vec2 cubicBezier(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3, float t);
    
    // Helper: Calculate path curvature at point i
    static float calculateCurvature(const std::vector<Vec2>& path, size_t i);
};
