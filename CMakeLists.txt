cmake_minimum_required(VERSION 3.10)
project(system_monitor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(/src/cpu)
include_directories(/src/memory)
add_executable (
    system_monitor
    # src/cpu/cpu_monitor.cpp
    src/memory/memory_monitor.cpp
)