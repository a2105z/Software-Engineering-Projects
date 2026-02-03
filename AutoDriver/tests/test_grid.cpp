#include <gtest/gtest.h>
#include "core/grid.h"

class GridTest : public ::testing::Test {
protected:
    void SetUp() override {
        grid = std::make_unique<Grid>(10, 10);
    }
    
    std::unique_ptr<Grid> grid;
};

TEST_F(GridTest, InitializedEmpty) {
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
            EXPECT_FALSE(grid->isObstacle(x, y));
        }
    }
}

TEST_F(GridTest, SetAndCheckObstacle) {
    grid->setObstacle(5, 5, true);
    EXPECT_TRUE(grid->isObstacle(5, 5));
    EXPECT_FALSE(grid->isObstacle(4, 5));
    EXPECT_FALSE(grid->isObstacle(6, 5));
}

TEST_F(GridTest, ToggleObstacle) {
    EXPECT_FALSE(grid->isObstacle(3, 3));
    grid->toggleObstacle(3, 3);
    EXPECT_TRUE(grid->isObstacle(3, 3));
    grid->toggleObstacle(3, 3);
    EXPECT_FALSE(grid->isObstacle(3, 3));
}

TEST_F(GridTest, ClearRemovesAllObstacles) {
    grid->setObstacle(1, 1, true);
    grid->setObstacle(5, 5, true);
    grid->setObstacle(9, 9, true);
    
    grid->clear();
    
    EXPECT_FALSE(grid->isObstacle(1, 1));
    EXPECT_FALSE(grid->isObstacle(5, 5));
    EXPECT_FALSE(grid->isObstacle(9, 9));
}

TEST_F(GridTest, BoundsChecking) {
    EXPECT_TRUE(grid->isValid(0, 0));
    EXPECT_TRUE(grid->isValid(9, 9));
    EXPECT_FALSE(grid->isValid(-1, 0));
    EXPECT_FALSE(grid->isValid(0, -1));
    EXPECT_FALSE(grid->isValid(10, 0));
    EXPECT_FALSE(grid->isValid(0, 10));
}

TEST_F(GridTest, OutOfBoundsTreatedAsObstacle) {
    EXPECT_TRUE(grid->isObstacle(-1, 5));
    EXPECT_TRUE(grid->isObstacle(15, 5));
}

TEST_F(GridTest, SetObstacleOutOfBoundsIgnored) {
    // Should not crash
    grid->setObstacle(-1, -1, true);
    grid->setObstacle(100, 100, true);
}
