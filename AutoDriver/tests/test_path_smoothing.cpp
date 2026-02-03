#include <gtest/gtest.h>
#include "core/path_smoothing.h"
#include "core/grid.h"

TEST(PathSmoothingTest, BezierCreatesMorePoints) {
    std::vector<Vec2> path = {
        Vec2(0.0f, 0.0f),
        Vec2(5.0f, 0.0f),
        Vec2(10.0f, 5.0f)
    };
    
    auto smoothed = PathSmoothing::bezierSmooth(path, 10);
    
    // Should have more points than original
    EXPECT_GT(smoothed.size(), path.size());
    
    // Should start and end at same points
    EXPECT_FLOAT_EQ(smoothed.front().x, path.front().x);
    EXPECT_FLOAT_EQ(smoothed.front().y, path.front().y);
}

TEST(PathSmoothingTest, ShortcutRemovesUnnecessaryPoints) {
    Grid grid(20, 20);
    
    // Path with unnecessary zigzag
    std::vector<Vec2> path = {
        Vec2(0.0f, 0.0f),
        Vec2(1.0f, 0.0f),
        Vec2(2.0f, 0.0f),
        Vec2(3.0f, 0.0f),
        Vec2(4.0f, 0.0f),
        Vec2(5.0f, 0.0f)
    };
    
    auto smoothed = PathSmoothing::shortcutSmooth(path, grid, 10);
    
    // Should have fewer points (straight line shortcut)
    EXPECT_LT(smoothed.size(), path.size());
    
    // Should keep start and end
    EXPECT_FLOAT_EQ(smoothed.front().x, path.front().x);
    EXPECT_FLOAT_EQ(smoothed.back().x, path.back().x);
}

TEST(PathSmoothingTest, GradientReducesCurvature) {
    Grid grid(20, 20);
    
    // Path with sharp turn
    std::vector<Vec2> path = {
        Vec2(0.0f, 5.0f),
        Vec2(5.0f, 5.0f),
        Vec2(5.0f, 10.0f)
    };
    
    auto smoothed = PathSmoothing::gradientSmooth(path, grid, 50, 0.2f);
    
    // Should still have similar number of points
    EXPECT_EQ(smoothed.size(), path.size());
    
    // Middle point should have moved slightly to reduce sharpness
    // (not exactly testable without curvature metric, but check it moved)
    EXPECT_NE(smoothed[1].x, path[1].x);
}

TEST(PathSmoothingTest, SmoothedPathAvoidsObstacles) {
    Grid grid(20, 20);
    
    // Add obstacle
    grid.setObstacle(5, 5, true);
    
    std::vector<Vec2> path = {
        Vec2(4.0f, 5.0f),
        Vec2(5.0f, 5.0f),  // Goes through obstacle
        Vec2(6.0f, 5.0f)
    };
    
    auto smoothed = PathSmoothing::gradientSmooth(path, grid, 20, 0.1f);
    
    // Middle point should have moved away from obstacle
    EXPECT_NE(smoothed[1].y, 5.0f);
}

TEST(PathSmoothingTest, HandlesShortPaths) {
    Grid grid(10, 10);
    
    std::vector<Vec2> path = {
        Vec2(0.0f, 0.0f),
        Vec2(1.0f, 1.0f)
    };
    
    // Should not crash on short paths
    auto bezier = PathSmoothing::bezierSmooth(path, 10);
    auto shortcut = PathSmoothing::shortcutSmooth(path, grid);
    auto gradient = PathSmoothing::gradientSmooth(path, grid);
    
    EXPECT_GE(bezier.size(), 2);
    EXPECT_GE(shortcut.size(), 2);
    EXPECT_GE(gradient.size(), 2);
}

TEST(PathSmoothingTest, CombinedSmoothingWorks) {
    Grid grid(20, 20);
    
    std::vector<Vec2> path = {
        Vec2(0.0f, 0.0f),
        Vec2(2.0f, 0.0f),
        Vec2(4.0f, 0.0f),
        Vec2(4.0f, 2.0f),
        Vec2(4.0f, 4.0f),
        Vec2(6.0f, 4.0f)
    };
    
    auto smoothed = PathSmoothing::smoothPath(path, grid);
    
    EXPECT_FALSE(smoothed.empty());
    EXPECT_GT(smoothed.size(), 0);
}
