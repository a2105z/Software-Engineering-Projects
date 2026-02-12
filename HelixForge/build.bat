@echo off
REM HelixForge Build Script for Windows

setlocal

set BUILD_TYPE=Release
if not "%1"=="" set BUILD_TYPE=%1

set BUILD_DIR=build

echo ===================================
echo   HelixForge Build Script
echo ===================================
echo Build Type: %BUILD_TYPE%
echo.

REM Create build directory
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
cd %BUILD_DIR%

REM Configure
echo Configuring with CMake...
cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE%

REM Build
echo.
echo Building...
cmake --build . --config %BUILD_TYPE%

echo.
echo ===================================
echo Build complete!
echo ===================================
echo Executable: %BUILD_DIR%\%BUILD_TYPE%\helixforge.exe
echo.
echo Run tests:
echo   helixforge.exe --version
echo.
echo See QUICKSTART.md for usage examples

endlocal
