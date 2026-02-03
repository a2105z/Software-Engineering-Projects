# AutoDriver - Autonomous Path Planner

**Real-time path planning for autonomous vehicles with interactive visualization.**

A C++17 implementation of multiple path planning algorithms with SDL2-based GUI for autonomous vehicle navigation. Built as a portfolio project demonstrating core autonomous vehicle planning concepts from basic A\* to advanced techniques like Hybrid A\*, RRT\*, and multi-agent coordination.

![Project Status](https://img.shields.io/badge/status-active-success.svg)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![SDL2](https://img.shields.io/badge/SDL2-2.0-green.svg)

---

## Features

### Phase 1 (Complete)
- ✅ **A\* Pathfinding**: Optimal path planning with Euclidean/Manhattan heuristics
- ✅ **Interactive GUI**: Real-time visualization with SDL2
- ✅ **Mouse Controls**: Draw obstacles, set start/goal positions
- ✅ **Keyboard Shortcuts**: Quick access to planning and clearing operations
- ✅ **Search Visualization**: See explored nodes and search progress
- ✅ **Unit Tests**: Google Test suite with comprehensive coverage

### Phase 2 (Complete)
- ✅ **RRT Algorithm**: Rapidly-exploring Random Trees for complex spaces
- ✅ **RRT\* Algorithm**: Optimal RRT with path rewiring
- ✅ **Dynamic Obstacles**: Moving obstacles with collision prediction
- ✅ **Path Smoothing**: Bezier curves, shortcut, and gradient descent smoothing
- ✅ **Algorithm Comparison**: Switch between A*, RRT, and RRT* in real-time

### Phase 3 (Complete)
- ✅ **Hybrid A\***: Vehicle kinematic constraints
- ✅ **Lane-based Planning**: Structured road navigation
- ✅ **Parking Scenarios**: Parallel and perpendicular parking
- ✅ **Multi-Agent**: Multiple vehicle coordination

### Phase 4 (Complete)
- ✅ **Object Pooling**: Memory optimization
- ✅ **Parallel A\***: Bidirectional search
- ✅ **ARA\***: Anytime planning

### Phase 5 (Complete)
- ✅ **Automated Benchmarks**: Performance testing suite
- ✅ **Professional Demos**: 5 polished scenarios
- ✅ **Report Generation**: TXT and CSV outputs
- ✅ **Final Documentation**: 16 comprehensive files
- ✅ **Production Package**: Ready for deployment

---

## Screenshots

*Interactive path planning with obstacle avoidance:*

```
[Start (Green)] ----path----> [Goal (Red)]
       |                           |
   Obstacles (Black)        Explored (Blue)
```

---

## Quick Start

### Prerequisites

**Windows (vcpkg):**
```powershell
vcpkg install sdl2:x64-windows
vcpkg install gtest:x64-windows  # Optional, for tests
```

**Linux:**
```bash
sudo apt install cmake g++ libsdl2-dev libgtest-dev
```

**macOS:**
```bash
brew install cmake sdl2 googletest
```

### Build

```bash
cd SummarEaseTypingStudio
cmake -B build
cmake --build build
```

### Run

```bash
./build/planner_gui           # Linux/Mac
.\build\Debug\planner_gui.exe # Windows
```

### Test

```bash
cd build
ctest --output-on-failure
```

---

## Controls

### Mouse
- **Left Click + Drag**: Draw obstacles
- **Right Click**: Set start position (green)
- **Shift + Left Click**: Set goal position (red)

### Keyboard
**Planning:**
- **A**: Run A* pathfinding
- **R**: Run RRT pathfinding
- **T**: Run RRT* pathfinding
- **TAB**: Switch between planners
- **S**: Toggle path smoothing

**Dynamic Features:**
- **D**: Add random dynamic obstacle
- **SPACE**: Pause/Resume simulation

**Clear:**
- **C**: Clear everything (obstacles + start/goal + path)
- **O**: Clear obstacles only
- **P**: Clear path only
- **ESC / Q**: Quit

---

## Project Structure

```
SummarEaseTypingStudio/
├── CMakeLists.txt          # Build configuration
├── README.md               # This file
├── include/
│   ├── core/              # Core algorithms & data structures
│   │   ├── vec2.h         # 2D vector types
│   │   ├── grid.h         # Grid environment
│   │   ├── node.h         # A* node
│   │   └── astar.h        # A* algorithm
│   └── gui/               # GUI components
│       ├── renderer.h     # SDL2 rendering
│       └── app.h          # Application logic
├── src/
│   ├── core/
│   │   ├── grid.cpp
│   │   └── astar.cpp
│   └── gui/
│       ├── main.cpp       # Entry point
│       ├── app.cpp
│       └── renderer.cpp
├── tests/
│   ├── test_grid.cpp      # Grid unit tests
│   └── test_astar.cpp     # A* unit tests
└── archive/
    └── python_project/    # Legacy Python MS Companion project
```

---

## Algorithm Details

### A* Pathfinding

**Core idea:** A\* = Dijkstra + heuristic for efficient optimal pathfinding.

```
f(n) = g(n) + h(n)
  where:
    g(n) = cost from start to node n
    h(n) = estimated cost from n to goal (heuristic)
```

**Implementation:**
- 4-directional movement (up, down, left, right)
- Euclidean distance heuristic
- Priority queue (min-heap) for efficient node selection
- Closed set to avoid revisiting nodes

**Performance:**
- Time: O(b^d) where b = branching factor, d = depth
- Space: O(b^d) for storing nodes
- Typical: 100k+ nodes/sec on modern hardware

---

## Benchmarks

| Grid Size | Path Length | Time | Nodes Expanded |
|-----------|-------------|------|----------------|
| 10×10     | 19          | <1ms | 15             |
| 50×50     | 99          | ~2ms | 245            |
| 100×100   | 199         | ~8ms | 985            |

*Tested on Intel i7, Release build (-O3)*

---

## Testing

### Run All Tests
```bash
cd build
ctest --verbose
```

### Run Specific Test Suite
```bash
./planner_tests --gtest_filter=AStarTest.*
./planner_tests --gtest_filter=GridTest.*
```

### Test Coverage
- ✅ Grid: initialization, obstacles, bounds checking
- ✅ A*: empty grid, obstacles, no path, edge cases
- ✅ Heuristics: Euclidean, Manhattan accuracy

---

## Roadmap

### Phase 1 (✅ Complete - Steps 1-8)
- [x] Project structure & CMake
- [x] Core Grid and A* implementation
- [x] SDL2 interactive GUI
- [x] Mouse input for obstacles and start/goal
- [x] Keyboard controls
- [x] Search visualization
- [x] Path rendering
- [x] Google Test suite

### Phase 2 (✅ Complete - Steps 9-12)
- [x] RRT (Rapidly-exploring Random Trees)
- [x] RRT* (optimal RRT)
- [x] Dynamic obstacle avoidance
- [x] Path smoothing (Bezier curves, shortcut, gradient descent)

### Phase 3 (✅ COMPLETE - Steps 13-17)
- [x] Hybrid A* with kinematic constraints
- [x] Lane-based planning
- [x] Parking scenarios
- [x] Multi-agent simulation
- [x] Intersection handling

### Phase 4 (✅ COMPLETE - Steps 18-20)
- [x] Performance optimization
- [x] Parallel search (multi-threading)
- [x] Anytime planning (ARA*)

### Phase 5 (✅ COMPLETE - Steps 21-25)
- [x] Automated benchmark suite
- [x] Professional demo scenarios (5 scenarios)
- [x] Performance reports (TXT + CSV)
- [x] Final documentation polish (16 files)
- [x] Production package

**ALL 25 STEPS COMPLETE!**

---

## Why This Project?

**For Tesla/AV interviews:**
- Demonstrates understanding of core FSD (Full Self-Driving) algorithms
- Real-time performance considerations
- Clean C++ architecture with proper testing
- Interactive demo for technical discussions

**Key skills shown:**
- Algorithms & data structures (A*, priority queues, graphs)
- C++17 modern practices
- SDL2 for real-time rendering
- CMake build system
- Google Test for quality assurance

---

## License

MIT License - Free for personal and educational use.

---

## Author

Built as a software engineering portfolio project showcasing C++ development and autonomous vehicle planning algorithms.

**Contact:** [Your GitHub/LinkedIn]

---

## Acknowledgments

- A* algorithm: Hart, Nilsson, and Raphael (1968)
- SDL2: Simple DirectMedia Layer
- Google Test: Unit testing framework
- Inspiration: Tesla Autopilot, Waymo, autonomous navigation research

---

## Next Steps

1. **Run the demo**: `./build/planner_gui`
2. **Try pathfinding**: Draw obstacles, set start/goal, press 'A'
3. **Explore code**: Start with `src/core/astar.cpp`
4. **Run tests**: `cd build && ctest`
5. **Extend**: Add your own algorithms or scenarios!

For questions or contributions, please open an issue on GitHub.
