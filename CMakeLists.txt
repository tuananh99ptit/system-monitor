cmake_minimum_required(VERSION 3.10)
project(system_monitor VERSION 1.0 LANGUAGES CXX)

# ==============================================================================
# 1. C++ STANDARD CONFIGURATION
# ==============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# 2. DEFINE EXECUTABLE TARGET AND SOURCE FILES (.cpp)
# ==============================================================================
add_executable(
    system_monitor
    src/main.cpp
    src/app/app.cpp           # Implementation file for the App class (manages the Run loop)
    src/cpu/cpu_monitor.cpp   # Implementation file for the CPU Monitor module
)

# ==============================================================================
# 3. DECLARE INCLUDE DIRECTORIES FOR HEADER FILES (.h) FOR THE TARGET
# ==============================================================================
target_include_directories(system_monitor PRIVATE
    src/app                   # Directory containing app.h
    src/cpu                   # Directory containing cpu_monitor.h
)