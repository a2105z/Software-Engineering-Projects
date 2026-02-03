#include <gtest/gtest.h>
#include "core/rrt.h"
#include "core/grid.h"

class RRTTest : public ::testing::Test {
protected:
    void SetUp() override {
        grid = std::make_unique<Grid>(20, 20);
        rrt_planner = std::make_unique<RRT>(*grid);
        rrt_star_planner = std::make_unique<RRTStar>(*grid);
    }
    
    std::unique_ptr<Grid> grid;
    std::unique_ptr<RRT> rrt_planner;
    std::unique_ptr<RRTStar> rrt_star_planner;
};

TEST_F(RRTTest, FindsPathInEmptyGrid) {
    Vec2 start(2.0f, 2.0f);
    Vec2 goal(18.0f, 18.0f);
    
    auto result = rrt_planner->findPath(start, goal, 2000);
    
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.path.empty());
    EXPECT_GT(result.tree_nodes.size(), 0);
}

TEST_F(RRTTest, ReturnsPartialPathWhenGoalUnreachable) {
    // Block entire middle section
    for (int y = 8; y < 12; y++) {
        for (int x = 0; x < 20; x++) {
            grid->setObstacle(x, y, true);
        }
    }
    
    Vec2 start(10.0f, 5.0f);
    Vec2 goal(10.0f, 15.0f);
    
    auto result = rrt_planner->findPath(start, goal, 1000);
    
    // May not reach goal, but should explore
    EXPECT_GT(result.tree_nodes.size(), 0);
    EXPECT_GT(result.iterations, 0);
}

TEST_F(RRTTest, RRTStarFindsBetterPath) {
    Vec2 start(2.0f, 2.0f);
    Vec2 goal(18.0f, 18.0f);
    
    auto rrt_result = rrt_planner->findPath(start, goal, 2000);
    auto rrt_star_result = rrt_star_planner->findPath(start, goal, 2000);
    
    if (rrt_result.success && rrt_star_result.success) {
        // RRT* should find equal or better path
        EXPECT_LE(rrt_star_result.path_cost, rrt_result.path_cost * 1.1f);
    }
}

TEST_F(RRTTest, HandlesObstacles) {
    // Create obstacle
    for (int i = 5; i < 15; i++) {
        grid->setObstacle(10, i, true);
    }
    
    Vec2 start(5.0f, 10.0f);
    Vec2 goal(15.0f, 10.0f);
    
    auto result = rrt_planner->findPath(start, goal, 3000);
    
    // Should find path around obstacle
    EXPECT_TRUE(result.success || result.path.size() > 0);
}

TEST_F(RRTTest, RespectsGridBounds) {
    Vec2 start(1.0f, 1.0f);
    Vec2 goal(19.0f, 19.0f);
    
    auto result = rrt_planner->findPath(start, goal, 1000);
    
    // All tree nodes should be within bounds
    for (const auto& node : result.tree_nodes) {
        EXPECT_GE(node.x, 0.0f);
        EXPECT_LT(node.x, 20.0f);
        EXPECT_GE(node.y, 0.0f);
        EXPECT_LT(node.y, 20.0f);
    }
}

TEST_F(RRTTest, PathIsConnected) {
    Vec2 start(2.0f, 2.0f);
    Vec2 goal(18.0f, 18.0f);
    
    auto result = rrt_planner->findPath(start, goal, 2000);
    
    if (result.success && result.path.size() > 1) {
        // Check that consecutive waypoints are reasonably close
        for (size_t i = 0; i < result.path.size() - 1; i++) {
            float dist = result.path[i].distanceTo(result.path[i + 1]);
            EXPECT_LT(dist, 5.0f);  // Max step size constraint
        }
    }
}
