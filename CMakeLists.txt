cmake_minimum_required(VERSION 3.10)
project(system_monitor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# 1. ADD SUBDIRECTORIES (MODULES)
# ==============================================================================
add_subdirectory(src/common)
add_subdirectory(src/cpu)
add_subdirectory(src/memory)
add_subdirectory(src/disk)
add_subdirectory(src/process)
add_subdirectory(src/app)

# ==============================================================================
# 2. CREATE MAIN APPLICATION EXECUTABLE
# ==============================================================================
add_executable(system_monitor src/main.cpp)
target_link_libraries(system_monitor PRIVATE app_lib)

# ==============================================================================
# 3. CREATE UNIT TEST TARGETS (IN TESTS/ DIRECTORY)
# ==============================================================================
add_executable(test_common tests/test_common.cpp)
target_link_libraries(test_common PRIVATE common)

add_executable(test_logger tests/test_logger.cpp)
target_link_libraries(test_logger PRIVATE app_lib)