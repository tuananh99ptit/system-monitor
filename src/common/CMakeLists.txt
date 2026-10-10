cmake_minimum_required(VERSION 3.10)
project(system_monitor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# 1. CREATE STATIC LIBRARY: libmonitor.a
# ==============================================================================
add_library(
    common STATIC
    logger/logger.cpp
    file_reader/file_reader.cpp
    utils/utils.cpp
)

# Declare header directories directly associated with the 'monitor' target
target_include_directories(
    common PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)
# ==============================================================================
# 2. CREATE MAIN APPLICATION EXECUTABLE
# ==============================================================================


# ==============================================================================
# 3. CREATE UNIT TEST TARGETS (IN TESTS/ DIRECTORY)
# ==============================================================================


