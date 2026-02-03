cmake_minimum_required(VERSION 3.15)
project(AutoDriver CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags for performance
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /O2")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3 -march=native")
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Core library (algorithms and data structures)
add_library(planner_core STATIC
    src/core/grid.cpp
    src/core/astar.cpp
    src/core/rrt.cpp
    src/core/dynamic_obstacle.cpp
    src/core/path_smoothing.cpp
    src/core/hybrid_astar.cpp
    src/core/lane_planner.cpp
    src/core/parking_planner.cpp
    src/core/multi_agent.cpp
    src/core/performance_optimizer.cpp
)

# Benchmark library
add_library(benchmark_lib STATIC
    src/benchmark/benchmark_suite.cpp
)

target_link_libraries(benchmark_lib planner_core)

# Demo executables (no SDL2 required)
add_executable(demo demo.cpp)
target_link_libraries(demo planner_core)

add_executable(demo_phase2 demo_phase2.cpp)
target_link_libraries(demo_phase2 planner_core)

add_executable(demo_phase3_4 demo_phase3_4.cpp)
target_link_libraries(demo_phase3_4 planner_core)

add_executable(demo_final demo_final.cpp)
target_link_libraries(demo_final planner_core benchmark_lib)

add_executable(benchmark benchmark_runner.cpp)
target_link_libraries(benchmark planner_core benchmark_lib)

message(STATUS "Demos built:")
message(STATUS "  - Phase 1: ./build/demo or .\\build\\Release\\demo.exe")
message(STATUS "  - Phase 2: ./build/demo_phase2 or .\\build\\Release\\demo_phase2.exe")
message(STATUS "  - Phase 3-4: ./build/demo_phase3_4 or .\\build\\Release\\demo_phase3_4.exe")
message(STATUS "  - Final: ./build/demo_final or .\\build\\Release\\demo_final.exe")
message(STATUS "  - Benchmark: ./build/benchmark or .\\build\\Release\\benchmark.exe")

# Try to find SDL2 for GUI
find_package(SDL2 QUIET)

if(SDL2_FOUND)
    # GUI executable
    add_executable(planner_gui
        src/gui/main.cpp
        src/gui/app.cpp
        src/gui/renderer.cpp
    )
    
    target_include_directories(planner_gui PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(planner_gui 
        planner_core
        ${SDL2_LIBRARIES}
    )
    
    message(STATUS "SDL2 found - GUI enabled")
    message(STATUS "Run with: ./build/planner_gui")
else()
    message(STATUS "SDL2 not found - GUI disabled")
    message(STATUS "Install SDL2:")
    message(STATUS "  Windows: vcpkg install sdl2:x64-windows")
    message(STATUS "  Linux:   sudo apt install libsdl2-dev")
    message(STATUS "  macOS:   brew install sdl2")
    message(STATUS "")
    message(STATUS "Core library and tests will still build.")
endif()

# Enable testing
enable_testing()

# Find Google Test
find_package(GTest QUIET)

if(GTest_FOUND)
    # Test executable
    add_executable(planner_tests
        tests/test_grid.cpp
        tests/test_astar.cpp
        tests/test_rrt.cpp
        tests/test_dynamic_obstacles.cpp
        tests/test_path_smoothing.cpp
    )
    
    target_link_libraries(planner_tests
        planner_core
        GTest::gtest
        GTest::gtest_main
    )
    
    # Register tests with CTest
    add_test(NAME GridTests COMMAND planner_tests --gtest_filter=GridTest.*)
    add_test(NAME AStarTests COMMAND planner_tests --gtest_filter=AStarTest.*)
    add_test(NAME RRTTests COMMAND planner_tests --gtest_filter=RRTTest.*)
    add_test(NAME DynamicObstacleTests COMMAND planner_tests --gtest_filter=DynamicObstacleTest.*)
    add_test(NAME PathSmoothingTests COMMAND planner_tests --gtest_filter=PathSmoothingTest.*)
    
    message(STATUS "Google Test found - tests enabled")
else()
    message(STATUS "Google Test not found - tests disabled")
    message(STATUS "Install with: vcpkg install gtest (Windows) or apt install libgtest-dev (Linux)")
endif()

# Print build configuration
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ flags: ${CMAKE_CXX_FLAGS}")
