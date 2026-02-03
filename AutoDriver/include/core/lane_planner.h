#pragma once

#include <vector>
#include "vec2.h"
#include "grid.h"

/**
 * Lane representation for structured road planning.
 */
struct Lane {
    int id;
    std::vector<Vec2> centerline;  // Waypoints defining lane center
    float width;
    float speed_limit;
    std::vector<int> left_lanes;   // IDs of lanes to the left
    std::vector<int> right_lanes;  // IDs of lanes to the right
    
    Lane(int lane_id, float w = 3.5f, float speed = 25.0f)
        : id(lane_id), width(w), speed_limit(speed) {}
};

/**
 * Lane change maneuver.
 */
struct LaneChangeManeuver {
    int from_lane;
    int to_lane;
    float start_s;  // Start position along lane (arc length)
    float duration; // Time to complete lane change
    
    LaneChangeManeuver(int from, int to, float s, float dur)
        : from_lane(from), to_lane(to), start_s(s), duration(dur) {}
};

/**
 * Lane-based path (sequence of lanes and maneuvers).
 */
struct LanePath {
    std::vector<int> lane_sequence;
    std::vector<LaneChangeManeuver> lane_changes;
    float total_cost;
    std::vector<Vec2> waypoints;  // Actual path waypoints
    
    LanePath() : total_cost(0.0f) {}
};

/**
 * Lane-based planner for highway/structured road scenarios.
 */
class LanePlanner {
public:
    explicit LanePlanner(const Grid& grid);
    
    // Add lane to road network
    void addLane(const Lane& lane);
    
    // Find lane-based path from start to goal
    LanePath findPath(Vec2 start, Vec2 goal);
    
    // Generate waypoints for following a lane
    std::vector<Vec2> generateLaneFollowingPath(int lane_id, float start_s, float end_s);
    
    // Generate lane change trajectory
    std::vector<Vec2> generateLaneChangeTrajectory(const LaneChangeManeuver& maneuver);
    
private:
    const Grid& grid_;
    std::vector<Lane> lanes_;
    
    // Find which lane a point is closest to
    int findClosestLane(Vec2 pos) const;
    
    // Get position along lane at arc length s
    Vec2 getLanePosition(int lane_id, float s) const;
    
    // Calculate cost of being in a lane
    float getLaneCost(int lane_id, float s) const;
};
