cmake_minimum_required(VERSION 3.10)
project(system_monitor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    src/cpu
    src/memory
    src/disk
    src/process
)

# 1. Tạo Static Library chứa 4 module (CMake sẽ sinh ra file libmonitor.a)
add_library(
    monitor STATIC
    src/cpu/cpu_monitor.cpp
    src/memory/memory_monitor.cpp
    src/disk/disk_monitor.cpp
    src/process/process_monitor.cpp
)

# 2. Tạo executable main và link tới thư viện monitor vừa tạo
add_executable(system_monitor src/main.cpp)
target_link_libraries(system_monitor PRIVATE monitor)