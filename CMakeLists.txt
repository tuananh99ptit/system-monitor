cmake_minimum_required(VERSION 3.10)
project(system_monitor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(../src/cpu)
add_executable(
    system_monitor
    ..src/main.cpp
    ..src/cpu/cpu_monitor.cpp
)
