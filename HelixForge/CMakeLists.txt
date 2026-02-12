cmake_minimum_required(VERSION 3.14)
project(HelixForge VERSION 1.0.0 LANGUAGES CXX)

# C++17 required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Options
option(BUILD_PYTHON_BINDINGS "Build Python bindings with pybind11" OFF)
option(BUILD_TESTS "Build test suite" OFF)
option(USE_SQLITE "Enable SQLite experiment logging" OFF)

# =============================================================================
# Core Library
# =============================================================================

set(HELIXFORGE_SOURCES
    # Phase 1: Core
    src/phase1_core/sequence.cpp
    
    # Phase 2: Evaluation
    src/phase2_evaluation/scoring.cpp
    src/phase2_evaluation/metrics.cpp
    src/phase2_evaluation/constraint_engine.cpp
    
    # Phase 3: Mutation
    src/phase3_mutation/rng.cpp
    src/phase3_mutation/mutation_engine.cpp
    
    # Phase 4: Optimization
    src/phase4_optimization/optimizer.cpp
    src/phase4_optimization/hill_climbing.cpp
    src/phase4_optimization/simulated_annealing.cpp
    src/phase4_optimization/beam_search.cpp
    src/phase4_optimization/genetic_algorithm.cpp
    
    # Phase 5: Interface
    src/phase5_interface/cli.cpp
    src/phase5_interface/json_output.cpp
    src/phase5_interface/experiment_log.cpp
)

# Create static library
add_library(helixforge_lib STATIC ${HELIXFORGE_SOURCES})

target_include_directories(helixforge_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# SQLite support (optional)
if(USE_SQLITE)
    find_package(SQLite3 REQUIRED)
    target_link_libraries(helixforge_lib PUBLIC SQLite::SQLite3)
    target_compile_definitions(helixforge_lib PUBLIC USE_SQLITE)
endif()

# =============================================================================
# Executable
# =============================================================================

add_executable(helixforge app/main.cpp)
target_link_libraries(helixforge PRIVATE helixforge_lib)

# =============================================================================
# Python Bindings (Optional)
# =============================================================================

if(BUILD_PYTHON_BINDINGS)
    find_package(pybind11 CONFIG REQUIRED)
    
    pybind11_add_module(pyhelixforge bindings/python/helixforge_bindings.cpp)
    target_link_libraries(pyhelixforge PRIVATE helixforge_lib)
    target_compile_definitions(pyhelixforge PRIVATE USE_PYBIND11)
endif()

# =============================================================================
# Installation
# =============================================================================

install(TARGETS helixforge
    RUNTIME DESTINATION bin
)

install(TARGETS helixforge_lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY src/
    DESTINATION include/helixforge
    FILES_MATCHING PATTERN "*.hpp"
)

# =============================================================================
# Tests (Optional)
# =============================================================================

if(BUILD_TESTS)
    enable_testing()
    # Add test subdirectory here if you create tests
endif()

# =============================================================================
# Summary
# =============================================================================

message(STATUS "")
message(STATUS "HelixForge Configuration:")
message(STATUS "  Version:           ${PROJECT_VERSION}")
message(STATUS "  C++ Standard:      C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "  Python Bindings:   ${BUILD_PYTHON_BINDINGS}")
message(STATUS "  SQLite Support:    ${USE_SQLITE}")
message(STATUS "  Tests:             ${BUILD_TESTS}")
message(STATUS "")
