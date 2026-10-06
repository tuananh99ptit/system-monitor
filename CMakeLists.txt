cmake_minimum_required(VERSION 3.10)
project(system_monitor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# 1. CREATE STATIC LIBRARY: libmonitor.a
# ==============================================================================
add_library(
    monitor STATIC
    src/process/process_monitor.cpp
    src/app/app.cpp
)

# Declare header directories directly associated with the 'monitor' target
target_include_directories(
    monitor PUBLIC
    src/process
    src/app
)
# ==============================================================================
# 2. CREATE MAIN APPLICATION EXECUTABLE
# ==============================================================================
add_executable(system_monitor src/main.cpp)
target_link_libraries(system_monitor PRIVATE monitor)