#include <gtest/gtest.h>
#include "core/astar.h"
#include "core/grid.h"

class AStarTest : public ::testing::Test {
protected:
    void SetUp() override {
        grid = std::make_unique<Grid>(10, 10);
        planner = std::make_unique<AStar>(*grid);
    }
    
    std::unique_ptr<Grid> grid;
    std::unique_ptr<AStar> planner;
};

TEST_F(AStarTest, FindsPathInEmptyGrid) {
    auto result = planner->findPath(Vec2i(0, 0), Vec2i(9, 9));
    
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.path.empty());
    EXPECT_EQ(result.path.front(), Vec2i(0, 0));
    EXPECT_EQ(result.path.back(), Vec2i(9, 9));
    EXPECT_GT(result.nodes_expanded, 0);
}

TEST_F(AStarTest, StartEqualsGoal) {
    auto result = planner->findPath(Vec2i(5, 5), Vec2i(5, 5));
    
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.path.size(), 1);
    EXPECT_EQ(result.path[0], Vec2i(5, 5));
}

TEST_F(AStarTest, NoPathWithWallBlocking) {
    // Create vertical wall blocking path
    for (int y = 0; y < 10; y++) {
        grid->setObstacle(5, y, true);
    }
    
    auto result = planner->findPath(Vec2i(0, 5), Vec2i(9, 5));
    
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.path.empty());
}

TEST_F(AStarTest, FindsPathAroundObstacle) {
    // Create small obstacle in the middle
    grid->setObstacle(5, 5, true);
    
    auto result = planner->findPath(Vec2i(0, 0), Vec2i(9, 9));
    
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.path.empty());
    
    // Path should avoid the obstacle
    for (const auto& pos : result.path) {
        EXPECT_FALSE(pos.x == 5 && pos.y == 5);
    }
}

TEST_F(AStarTest, PathLengthReasonable) {
    // In empty 10x10 grid, Manhattan distance from (0,0) to (9,9) is 18
    auto result = planner->findPath(Vec2i(0, 0), Vec2i(9, 9));
    
    EXPECT_TRUE(result.success);
    // Path length should be Manhattan distance + 1 (for 4-directional)
    EXPECT_EQ(result.path.size(), 19);  // 9 steps right + 9 steps down + start cell
}

TEST_F(AStarTest, InvalidStartReturnsFailure) {
    grid->setObstacle(0, 0, true);
    auto result = planner->findPath(Vec2i(0, 0), Vec2i(9, 9));
    
    EXPECT_FALSE(result.success);
}

TEST_F(AStarTest, InvalidGoalReturnsFailure) {
    grid->setObstacle(9, 9, true);
    auto result = planner->findPath(Vec2i(0, 0), Vec2i(9, 9));
    
    EXPECT_FALSE(result.success);
}

TEST_F(AStarTest, OutOfBoundsReturnsFailure) {
    auto result1 = planner->findPath(Vec2i(-1, 0), Vec2i(5, 5));
    auto result2 = planner->findPath(Vec2i(5, 5), Vec2i(100, 100));
    
    EXPECT_FALSE(result1.success);
    EXPECT_FALSE(result2.success);
}

TEST_F(AStarTest, EuclideanDistanceCorrect) {
    float dist = AStar::euclideanDistance(Vec2i(0, 0), Vec2i(3, 4));
    EXPECT_FLOAT_EQ(dist, 5.0f);  // 3-4-5 triangle
}

TEST_F(AStarTest, ManhattanDistanceCorrect) {
    float dist = AStar::manhattanDistance(Vec2i(0, 0), Vec2i(3, 4));
    EXPECT_FLOAT_EQ(dist, 7.0f);  // 3 + 4
}

TEST_F(AStarTest, VisitedNodesTracked) {
    auto result = planner->findPath(Vec2i(0, 0), Vec2i(2, 2));
    
    EXPECT_TRUE(result.success);
    EXPECT_GT(result.visited.size(), 0);
    EXPECT_EQ(result.nodes_expanded, result.visited.size());
}
