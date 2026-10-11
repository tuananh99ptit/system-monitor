# ==============================================================================
# 1. CREATE STATIC LIBRARY: libdisk_lib.a
# ==============================================================================
add_library(
    disk_lib STATIC
    disk_monitor.cpp
)

# Declare header directories directly associated with the 'disk_lib' target
target_include_directories(
    disk_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies
target_link_libraries(disk_lib PUBLIC common)