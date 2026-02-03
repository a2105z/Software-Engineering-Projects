#include "core/parking_planner.h"
#include <cmath>

ParkingPlanner::ParkingPlanner(const Grid& grid, const VehicleParams& params)
    : hybrid_planner_(grid, params), vehicle_params_(params) {}

bool ParkingPlanner::isSpotAccessible(const ParkingSpot& spot) const {
    // Simplified check: spot should be large enough for vehicle
    return spot.width >= vehicle_params_.width + 0.5f &&
           spot.length >= vehicle_params_.length + 0.5f;
}

std::vector<HybridState> ParkingPlanner::generateParkingWaypoints(
    Vec2 start, float start_theta, const ParkingSpot& spot, bool parallel) {
    
    std::vector<HybridState> waypoints;
    
    if (parallel) {
        // Parallel parking: approach, align, reverse in
        waypoints.emplace_back(start, start_theta);
        waypoints.emplace_back(spot.center + Vec2(-spot.length/2, 2.0f), start_theta);
        waypoints.emplace_back(spot.center + Vec2(-spot.length/2, 1.0f), start_theta + 0.5f);
        waypoints.emplace_back(spot.center, spot.angle);
    } else {
        // Perpendicular parking: approach, turn, reverse in
        waypoints.emplace_back(start, start_theta);
        waypoints.emplace_back(spot.center + Vec2(0, spot.length), start_theta);
        waypoints.emplace_back(spot.center + Vec2(0, spot.length/2), spot.angle);
        waypoints.emplace_back(spot.center, spot.angle);
    }
    
    return waypoints;
}

ParkingManeuver ParkingPlanner::planParallelParking(
    Vec2 start, float start_theta, const ParkingSpot& spot) {
    
    ParkingManeuver maneuver;
    
    if (!isSpotAccessible(spot)) {
        return maneuver;
    }
    
    // Generate simplified parallel parking path
    maneuver.path = generateParkingWaypoints(start, start_theta, spot, true);
    maneuver.success = !maneuver.path.empty();
    maneuver.num_reversals = 2;  // Typical for parallel parking
    maneuver.total_cost = static_cast<float>(maneuver.path.size()) * 2.0f;
    
    return maneuver;
}

ParkingManeuver ParkingPlanner::planPerpendicularParking(
    Vec2 start, float start_theta, const ParkingSpot& spot) {
    
    ParkingManeuver maneuver;
    
    if (!isSpotAccessible(spot)) {
        return maneuver;
    }
    
    // Generate simplified perpendicular parking path
    maneuver.path = generateParkingWaypoints(start, start_theta, spot, false);
    maneuver.success = !maneuver.path.empty();
    maneuver.num_reversals = 1;  // Typical for perpendicular parking
    maneuver.total_cost = static_cast<float>(maneuver.path.size()) * 1.5f;
    
    return maneuver;
}
