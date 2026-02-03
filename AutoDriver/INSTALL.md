# Installation Guide

Complete setup instructions for building the Autonomous Path Planner.

---

## Prerequisites

### Required
- **CMake** 3.15 or higher
- **C++17 compiler**:
  - Windows: Visual Studio 2019+ or MinGW
  - Linux: GCC 7+ or Clang 5+
  - macOS: Xcode Command Line Tools

### Optional (for GUI)
- **SDL2** 2.0+ - Required for interactive visualization

### Optional (for tests)
- **Google Test** - Required for unit tests

---

## Installation Steps

### Windows (Recommended: vcpkg)

**1. Install vcpkg (if not already installed):**
```powershell
cd C:\
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
```

**2. Install dependencies:**
```powershell
.\vcpkg install sdl2:x64-windows
.\vcpkg install gtest:x64-windows
```

**3. Build the project:**
```powershell
cd c:\Users\aarav\OneDrive\Desktop\SummarEaseTypingStudio
cmake -B build -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
```

**4. Run:**
```powershell
.\build\Release\planner_gui.exe
```

---

### Linux (Ubuntu/Debian)

**1. Install dependencies:**
```bash
sudo apt update
sudo apt install cmake g++ libsdl2-dev libgtest-dev
```

**2. Build:**
```bash
cd ~/SummarEaseTypingStudio
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

**3. Run:**
```bash
./build/planner_gui
```

---

### macOS

**1. Install Homebrew (if not already installed):**
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

**2. Install dependencies:**
```bash
brew install cmake sdl2 googletest
```

**3. Build:**
```bash
cd ~/SummarEaseTypingStudio
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

**4. Run:**
```bash
./build/planner_gui
```

---

## Troubleshooting

### SDL2 not found

**Symptoms:**
```
CMake Error at CMakeLists.txt:X (find_package):
  Could not find a package configuration file provided by "SDL2"
```

**Solutions:**

**Windows:**
1. Use vcpkg (recommended):
   ```powershell
   vcpkg install sdl2:x64-windows
   cmake -B build -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
   ```

2. Or download SDL2 manually from [libsdl.org](https://www.libsdl.org/download-2.0.php) and set:
   ```powershell
   cmake -B build -DSDL2_DIR="C:\path\to\SDL2\cmake"
   ```

**Linux:**
```bash
sudo apt install libsdl2-dev
# or
sudo yum install SDL2-devel
```

**macOS:**
```bash
brew install sdl2
```

### Google Test not found

The project will build without tests if GTest is not found. Tests are optional.

**To enable tests:**

**Windows:**
```powershell
vcpkg install gtest:x64-windows
```

**Linux:**
```bash
sudo apt install libgtest-dev
```

**macOS:**
```bash
brew install googletest
```

### Build fails on Windows with "Cannot open include file"

Make sure you're using the vcpkg toolchain:
```powershell
cmake -B build -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

### SDL2.dll not found when running

**Windows only:** Copy SDL2.dll to the same directory as the executable:
```powershell
copy C:\vcpkg\installed\x64-windows\bin\SDL2.dll .\build\Release\
```

Or add vcpkg bin directory to PATH.

---

## Build Without GUI

If you don't want to install SDL2, you can build just the core library:

```bash
cmake -B build
cmake --build build
```

This builds:
- ✅ Core library (`planner_core`)
- ✅ Unit tests (if GTest is available)
- ❌ GUI application (requires SDL2)

The core algorithms work without SDL2.

---

## Verify Installation

After building, verify everything works:

**1. Check build outputs:**
```bash
ls build/Release/        # Windows
ls build/                # Linux/Mac
```

You should see:
- `planner_core.lib` / `libplanner_core.a` - Core library ✅
- `planner_gui.exe` / `planner_gui` - GUI app (if SDL2 installed) ✅
- `planner_tests.exe` / `planner_tests` - Tests (if GTest installed) ✅

**2. Run tests (if available):**
```bash
cd build
ctest --output-on-failure
```

Expected output:
```
Test project .../build
    Start 1: GridTests
1/2 Test #1: GridTests ........................   Passed    0.01 sec
    Start 2: AStarTests
2/2 Test #2: AStarTests .......................   Passed    0.02 sec

100% tests passed, 0 tests failed out of 2
```

**3. Run GUI (if SDL2 installed):**
```bash
./build/planner_gui           # Linux/Mac
.\build\Release\planner_gui.exe  # Windows
```

You should see a window with a 50×50 grid.

---

## Next Steps

Once everything builds:
1. ✅ Run the GUI and test pathfinding
2. ✅ Read the [README.md](README.md) for controls
3. ✅ Explore the code in `src/core/`
4. ✅ Run the tests to understand the API

For development:
- Build in Debug mode: `-DCMAKE_BUILD_TYPE=Debug`
- Enable verbose output: `cmake --build build --verbose`
- Clean build: `rm -rf build` then rebuild

---

## Support

If you encounter issues:
1. Check this troubleshooting guide
2. Verify all prerequisites are installed
3. Check CMake output for specific errors
4. Make sure you're using the correct CMake toolchain (vcpkg on Windows)

For SDL2/GTest installation issues, consult:
- SDL2: https://wiki.libsdl.org/Installation
- vcpkg: https://github.com/Microsoft/vcpkg
- Google Test: https://github.com/google/googletest
