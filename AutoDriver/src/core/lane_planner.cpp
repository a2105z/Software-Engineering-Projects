#include "core/lane_planner.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

LanePlanner::LanePlanner(const Grid& grid) : grid_(grid) {}

void LanePlanner::addLane(const Lane& lane) {
    lanes_.push_back(lane);
}

int LanePlanner::findClosestLane(Vec2 pos) const {
    int closest = -1;
    float min_dist = std::numeric_limits<float>::max();
    
    for (size_t i = 0; i < lanes_.size(); i++) {
        for (const auto& wp : lanes_[i].centerline) {
            float dist = pos.distanceTo(wp);
            if (dist < min_dist) {
                min_dist = dist;
                closest = static_cast<int>(i);
            }
        }
    }
    
    return closest;
}

Vec2 LanePlanner::getLanePosition(int lane_id, float s) const {
    if (lane_id < 0 || lane_id >= static_cast<int>(lanes_.size())) {
        return Vec2(0, 0);
    }
    
    const auto& lane = lanes_[lane_id];
    if (lane.centerline.empty()) return Vec2(0, 0);
    
    // Simple interpolation along waypoints
    size_t idx = static_cast<size_t>(s) % lane.centerline.size();
    return lane.centerline[idx];
}

float LanePlanner::getLaneCost(int lane_id, float s) const {
    // Simplified cost: prefer middle lanes, penalize lane changes
    return 1.0f + std::abs(lane_id - static_cast<int>(lanes_.size()) / 2) * 0.5f;
}

LanePath LanePlanner::findPath(Vec2 start, Vec2 goal) {
    LanePath path;
    
    int start_lane = findClosestLane(start);
    int goal_lane = findClosestLane(goal);
    
    if (start_lane < 0 || goal_lane < 0) return path;
    
    // Simple lane sequence: stay in start lane, then change to goal lane
    path.lane_sequence.push_back(start_lane);
    if (start_lane != goal_lane) {
        path.lane_sequence.push_back(goal_lane);
        path.lane_changes.emplace_back(start_lane, goal_lane, 5.0f, 3.0f);
    }
    
    // Generate waypoints
    path.waypoints = generateLaneFollowingPath(start_lane, 0, 10);
    if (start_lane != goal_lane) {
        auto change_waypoints = generateLaneChangeTrajectory(path.lane_changes[0]);
        path.waypoints.insert(path.waypoints.end(), change_waypoints.begin(), change_waypoints.end());
    }
    
    path.total_cost = static_cast<float>(path.waypoints.size());
    return path;
}

std::vector<Vec2> LanePlanner::generateLaneFollowingPath(int lane_id, float start_s, float end_s) {
    std::vector<Vec2> waypoints;
    
    if (lane_id < 0 || lane_id >= static_cast<int>(lanes_.size())) {
        return waypoints;
    }
    
    const auto& lane = lanes_[lane_id];
    for (const auto& wp : lane.centerline) {
        waypoints.push_back(wp);
    }
    
    return waypoints;
}

std::vector<Vec2> LanePlanner::generateLaneChangeTrajectory(const LaneChangeManeuver& maneuver) {
    std::vector<Vec2> waypoints;
    
    // Simple S-curve for lane change
    for (int i = 0; i <= 10; i++) {
        float t = i / 10.0f;
        Vec2 from_pos = getLanePosition(maneuver.from_lane, maneuver.start_s + t * 5.0f);
        Vec2 to_pos = getLanePosition(maneuver.to_lane, maneuver.start_s + t * 5.0f);
        
        // Smooth interpolation
        float s = 0.5f * (1.0f - std::cos(t * M_PI));
        waypoints.push_back(from_pos + (to_pos - from_pos) * s);
    }
    
    return waypoints;
}
