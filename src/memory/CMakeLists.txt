cmake_minimum_required(VERSION 3.10)
project(system_monitor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# 1. CREATE STATIC LIBRARY: libmonitor.a
# ==============================================================================
add_library(
    memory_monitor STATIC
    memory_monitor.cpp
)

target_include_directories(
    memory_monitor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
# ==============================================================================
# 2. CREATE MAIN APPLICATION EXECUTABLE
# ==============================================================================
# memory_monitor using logger/utils from common
target_link_libraries(
    memory_monitor PRIVATE
    common
)
# ==============================================================================
# 3. CREATE UNIT TEST TARGETS (IN TESTS/ DIRECTORY)
# ==============================================================================


