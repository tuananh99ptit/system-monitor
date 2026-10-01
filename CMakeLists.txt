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

add_executable(
    system_monitor
    src/main.cpp
    src/cpu/cpu_monitor.cpp
    src/memory/memory_monitor.cpp
    src/disk/disk_monitor.cpp
    src/process/process_monitor.cpp
)