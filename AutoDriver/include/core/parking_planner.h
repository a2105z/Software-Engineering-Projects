#pragma once

#include <vector>
#include "vec2.h"
#include "hybrid_astar.h"

/**
 * Parking spot representation.
 */
struct ParkingSpot {
    Vec2 center;
    float width;
    float length;
    float angle;  // Orientation (radians)
    bool is_parallel;
    
    ParkingSpot(Vec2 c, float w, float l, float a, bool parallel = false)
        : center(c), width(w), length(l), angle(a), is_parallel(parallel) {}
};

/**
 * Parking maneuver result.
 */
struct ParkingManeuver {
    std::vector<HybridState> path;
    float total_cost;
    bool success;
    int num_reversals;  // Number of gear shifts
    
    ParkingManeuver() : total_cost(0.0f), success(false), num_reversals(0) {}
};

/**
 * Parking planner using Hybrid A* for parallel/perpendicular parking.
 */
class ParkingPlanner {
public:
    ParkingPlanner(const Grid& grid, const VehicleParams& params);
    
    // Plan parallel parking maneuver
    ParkingManeuver planParallelParking(Vec2 start, float start_theta, const ParkingSpot& spot);
    
    // Plan perpendicular parking maneuver
    ParkingManeuver planPerpendicularParking(Vec2 start, float start_theta, const ParkingSpot& spot);
    
    // Check if spot is accessible
    bool isSpotAccessible(const ParkingSpot& spot) const;
    
private:
    HybridAStar hybrid_planner_;
    VehicleParams vehicle_params_;
    
    // Generate intermediate waypoints for parking
    std::vector<HybridState> generateParkingWaypoints(Vec2 start, float start_theta,
                                                      const ParkingSpot& spot, bool parallel);
};
