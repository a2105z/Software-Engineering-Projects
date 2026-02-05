cmake_minimum_required(VERSION 3.14)
project(SmartSignal VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(smartsignal
  src/main.cpp
  src/config.cpp
  src/gpio.cpp
  src/debounce.cpp
  src/detection.cpp
  src/fsm.cpp
  src/arbiter.cpp
  src/logger.cpp
)

target_include_directories(smartsignal PRIVATE ${CMAKE_SOURCE_DIR}/include)
