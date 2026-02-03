#include <gtest/gtest.h>
#include "core/dynamic_obstacle.h"

TEST(DynamicObstacleTest, UpdatesPosition) {
    Vec2 initial_pos(5.0f, 5.0f);
    Vec2 velocity(1.0f, 0.5f);
    DynamicObstacle obs(initial_pos, velocity);
    
    obs.update(1.0f);  // 1 second
    
    Vec2 new_pos = obs.getPosition();
    EXPECT_FLOAT_EQ(new_pos.x, 6.0f);
    EXPECT_FLOAT_EQ(new_pos.y, 5.5f);
}

TEST(DynamicObstacleTest, PredictsPosition) {
    Vec2 initial_pos(0.0f, 0.0f);
    Vec2 velocity(2.0f, 3.0f);
    DynamicObstacle obs(initial_pos, velocity);
    
    Vec2 predicted = obs.predictPosition(2.5f);
    
    EXPECT_FLOAT_EQ(predicted.x, 5.0f);
    EXPECT_FLOAT_EQ(predicted.y, 7.5f);
}

TEST(DynamicObstacleTest, DetectsCollision) {
    Vec2 pos(10.0f, 10.0f);
    Vec2 velocity(0.0f, 0.0f);
    DynamicObstacle obs(pos, velocity, 1.0f);  // radius = 1
    
    EXPECT_TRUE(obs.collides(Vec2(10.5f, 10.5f), 0.0f));
    EXPECT_FALSE(obs.collides(Vec2(15.0f, 15.0f), 0.0f));
}

TEST(DynamicObstacleTest, DetectsPathCollision) {
    Vec2 pos(5.0f, 5.0f);
    Vec2 velocity(1.0f, 0.0f);
    DynamicObstacle obs(pos, velocity, 0.5f);
    
    // Path that intersects obstacle trajectory
    Vec2 from(4.0f, 5.0f);
    Vec2 to(8.0f, 5.0f);
    
    EXPECT_TRUE(obs.collidesWithPath(from, to, 0.0f, 1.0f));
}

TEST(DynamicObstacleManagerTest, ManagesMultipleObstacles) {
    DynamicObstacleManager manager;
    
    manager.addObstacle(DynamicObstacle(Vec2(1.0f, 1.0f), Vec2(0.5f, 0.0f)));
    manager.addObstacle(DynamicObstacle(Vec2(5.0f, 5.0f), Vec2(0.0f, 0.5f)));
    
    EXPECT_EQ(manager.size(), 2);
    
    manager.updateAll(1.0f);
    
    // Check obstacles moved
    EXPECT_FLOAT_EQ(manager.getObstacles()[0].getPosition().x, 1.5f);
    EXPECT_FLOAT_EQ(manager.getObstacles()[1].getPosition().y, 5.5f);
}

TEST(DynamicObstacleManagerTest, ChecksCollisions) {
    DynamicObstacleManager manager;
    manager.addObstacle(DynamicObstacle(Vec2(10.0f, 10.0f), Vec2(0.0f, 0.0f), 1.0f));
    
    EXPECT_TRUE(manager.checkCollision(Vec2(10.5f, 10.5f), 0.0f));
    EXPECT_FALSE(manager.checkCollision(Vec2(20.0f, 20.0f), 0.0f));
}

TEST(DynamicObstacleManagerTest, ClearRemovesAll) {
    DynamicObstacleManager manager;
    manager.addObstacle(DynamicObstacle(Vec2(1.0f, 1.0f), Vec2(0.0f, 0.0f)));
    manager.addObstacle(DynamicObstacle(Vec2(2.0f, 2.0f), Vec2(0.0f, 0.0f)));
    
    EXPECT_EQ(manager.size(), 2);
    
    manager.clear();
    
    EXPECT_EQ(manager.size(), 0);
}
