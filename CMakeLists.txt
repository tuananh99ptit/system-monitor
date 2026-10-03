cmake_minimum_required(VERSION 3.10)
project(system_monitor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# 1. CREATE STATIC LIBRARY: libmonitor.a
# ==============================================================================
add_library(
    monitor STATIC
    src/common/logger/logger.cpp
    src/common/file_reader/file_reader.cpp
    src/common/utils/utils.cpp
    src/cpu/cpu_monitor.cpp
    src/memory/memory_monitor.cpp
    src/disk/disk_monitor.cpp
    src/process/process_monitor.cpp
)

# Declare header directories directly associated with the 'monitor' target
target_include_directories(
    monitor PUBLIC
    src/common/config
    src/common/logger
    src/common/file_reader
    src/common/utils
    src/cpu
    src/memory
    src/disk
    src/process
)

# ==============================================================================
# 2. CREATE MAIN APPLICATION EXECUTABLE
# ==============================================================================
add_executable(system_monitor src/main.cpp)
target_link_libraries(system_monitor PRIVATE monitor)

# ==============================================================================
# 3. CREATE UNIT TEST TARGETS (IN TESTS/ DIRECTORY)
# ==============================================================================
add_executable(test_common tests/test_common.cpp)
target_link_libraries(test_common PRIVATE monitor)

add_executable(test_logger tests/test_logger.cpp)
target_link_libraries(test_logger PRIVATE monitor)

