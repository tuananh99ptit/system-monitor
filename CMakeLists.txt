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
add_subdirectory(src/monitor)
add_subdirectory(src/ui)
add_subdirectory(src/signal)
# ==============================================================================
# 2. CREATE MAIN APPLICATION EXECUTABLE
# ==============================================================================
add_executable(system_monitor src/main.cpp)
target_link_libraries(system_monitor PRIVATE app_lib)
target_link_libraries(system_monitor PRIVATE monitor_lib ui_lib)
target_link_libraries(system_monitor PRIVATE monitor_lib signal_lib)
# ==============================================================================
# 3. CREATE UNIT TEST TARGETS (IN TESTS/ DIRECTORY)
# ==============================================================================
add_executable(test_common tests/test_common.cpp)
target_link_libraries(test_common PRIVATE common)

add_executable(test_logger tests/test_logger.cpp)
target_link_libraries(test_logger PRIVATE app_lib)

add_executable(test_monitor_data tests/test_monitor_data.cpp)   
target_link_libraries(test_monitor_data PRIVATE monitor_lib)

add_executable(test_monitor_worker tests/test_monitor_worker.cpp)
target_link_libraries(test_monitor_worker PRIVATE monitor_lib)

add_executable(test_real_data tests/test_real_data.cpp)
target_link_libraries(test_real_data PRIVATE monitor_lib ui_lib)

add_executable(test_shutdown tests/test_shutdown.cpp)
target_link_libraries(test_shutdown PRIVATE signal_lib)