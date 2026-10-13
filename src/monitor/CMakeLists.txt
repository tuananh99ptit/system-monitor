# ==============================================================================
# CREATE STATIC LIBRARY: libmonitor_lib.a
# ==============================================================================
find_package(Threads REQUIRED)

add_library(
    monitor_lib STATIC
    monitor_data.cpp
    monitor_worker.cpp
)

# Declare header directories directly associated with the 'monitor_lib' target
target_include_directories(
    monitor_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies
target_link_libraries(
monitor_lib PUBLIC 
Threads::Threads
cpu_lib
memory_lib
disk_lib
process_lib
common
)