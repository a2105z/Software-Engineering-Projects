#include "core/path_smoothing.h"
#include <cmath>
#include <algorithm>

// ============================================================================
// Bezier Smoothing
// ============================================================================

Vec2 PathSmoothing::cubicBezier(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3, float t) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;
    
    Vec2 point = p0 * uuu;
    point = point + p1 * (3.0f * uu * t);
    point = point + p2 * (3.0f * u * tt);
    point = point + p3 * ttt;
    
    return point;
}

std::vector<Vec2> PathSmoothing::bezierSmooth(const std::vector<Vec2>& path,
                                              int points_per_segment) {
    if (path.size() < 2) return path;
    if (path.size() == 2) return path;
    
    std::vector<Vec2> smoothed;
    smoothed.push_back(path[0]);
    
    for (size_t i = 0; i < path.size() - 1; i++) {
        Vec2 p0 = path[i];
        Vec2 p3 = path[i + 1];
        
        // Calculate control points
        Vec2 direction;
        if (i > 0) {
            direction = path[i + 1] - path[i - 1];
        } else {
            direction = path[i + 1] - path[i];
        }
        Vec2 p1 = p0 + direction * 0.25f;
        
        if (i < path.size() - 2) {
            direction = path[i + 2] - path[i];
        } else {
            direction = path[i + 1] - path[i];
        }
        Vec2 p2 = p3 - direction * 0.25f;
        
        // Generate points along Bezier curve
        for (int j = 1; j <= points_per_segment; j++) {
            float t = static_cast<float>(j) / points_per_segment;
            Vec2 point = cubicBezier(p0, p1, p2, p3, t);
            smoothed.push_back(point);
        }
    }
    
    return smoothed;
}

// ============================================================================
// Shortcut Smoothing
// ============================================================================

bool PathSmoothing::isLineCollisionFree(Vec2 from, Vec2 to, const Grid& grid) {
    Vec2 direction = to - from;
    float dist = direction.length();
    
    if (dist < 0.01f) return true;
    
    int num_checks = static_cast<int>(std::ceil(dist * 2.0f));
    for (int i = 0; i <= num_checks; i++) {
        float t = static_cast<float>(i) / num_checks;
        Vec2 point = from + direction * t;
        
        int gx = static_cast<int>(std::round(point.x));
        int gy = static_cast<int>(std::round(point.y));
        
        if (grid.isObstacle(gx, gy)) {
            return false;
        }
    }
    
    return true;
}

std::vector<Vec2> PathSmoothing::shortcutSmooth(const std::vector<Vec2>& path,
                                               const Grid& grid,
                                               int max_iterations) {
    if (path.size() < 3) return path;
    
    std::vector<Vec2> smoothed = path;
    
    for (int iter = 0; iter < max_iterations; iter++) {
        bool improved = false;
        
        for (size_t i = 0; i < smoothed.size(); i++) {
            for (size_t j = i + 2; j < smoothed.size(); j++) {
                if (isLineCollisionFree(smoothed[i], smoothed[j], grid)) {
                    // Can shortcut - remove intermediate points
                    smoothed.erase(smoothed.begin() + i + 1, smoothed.begin() + j);
                    improved = true;
                    break;
                }
            }
            if (improved) break;
        }
        
        if (!improved) break;
    }
    
    return smoothed;
}

// ============================================================================
// Gradient Descent Smoothing
// ============================================================================

float PathSmoothing::calculateCurvature(const std::vector<Vec2>& path, size_t i) {
    if (i == 0 || i >= path.size() - 1) return 0.0f;
    
    Vec2 prev = path[i - 1];
    Vec2 curr = path[i];
    Vec2 next = path[i + 1];
    
    Vec2 v1 = curr - prev;
    Vec2 v2 = next - curr;
    
    float len1 = v1.length();
    float len2 = v2.length();
    
    if (len1 < 0.01f || len2 < 0.01f) return 0.0f;
    
    // Angle between vectors
    float dot = (v1.x * v2.x + v1.y * v2.y) / (len1 * len2);
    dot = std::max(-1.0f, std::min(1.0f, dot));
    
    return 1.0f - dot;  // Higher value = sharper turn
}

std::vector<Vec2> PathSmoothing::gradientSmooth(const std::vector<Vec2>& path,
                                               const Grid& grid,
                                               int iterations,
                                               float alpha) {
    if (path.size() < 3) return path;
    
    std::vector<Vec2> smoothed = path;
    
    for (int iter = 0; iter < iterations; iter++) {
        std::vector<Vec2> new_path = smoothed;
        
        // Don't move start and end points
        for (size_t i = 1; i < smoothed.size() - 1; i++) {
            Vec2 prev = smoothed[i - 1];
            Vec2 curr = smoothed[i];
            Vec2 next = smoothed[i + 1];
            
            // Calculate gradient (direction to reduce curvature)
            Vec2 midpoint_prev = (prev + curr) * 0.5f;
            Vec2 midpoint_next = (curr + next) * 0.5f;
            Vec2 gradient = midpoint_prev + midpoint_next - curr * 2.0f;
            
            // Update position
            Vec2 new_pos = curr + gradient * alpha;
            
            // Check if new position is valid
            int gx = static_cast<int>(std::round(new_pos.x));
            int gy = static_cast<int>(std::round(new_pos.y));
            
            if (!grid.isObstacle(gx, gy) && grid.isValid(gx, gy)) {
                // Check if moving doesn't create collision
                if (isLineCollisionFree(prev, new_pos, grid) &&
                    isLineCollisionFree(new_pos, next, grid)) {
                    new_path[i] = new_pos;
                }
            }
        }
        
        smoothed = new_path;
    }
    
    return smoothed;
}

// ============================================================================
// Combined Smoothing
// ============================================================================

std::vector<Vec2> PathSmoothing::smoothPath(const std::vector<Vec2>& path,
                                           const Grid& grid) {
    if (path.size() < 2) return path;
    
    // Step 1: Remove unnecessary waypoints with shortcut
    auto smoothed = shortcutSmooth(path, grid, 10);
    
    // Step 2: Apply gradient descent to reduce sharp turns
    smoothed = gradientSmooth(smoothed, grid, 30, 0.15f);
    
    // Step 3: Final pass with Bezier for visual smoothness
    smoothed = bezierSmooth(smoothed, 5);
    
    return smoothed;
}
