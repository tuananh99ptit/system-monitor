# ==============================================================================
# 1. CREATE STATIC LIBRARY: libcpu_lib.a
# ==============================================================================
add_library(
    cpu_lib STATIC
    cpu_monitor.cpp
)

# Declare header directories directly associated with the 'cpu_lib' target
target_include_directories(
    cpu_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies
target_link_libraries(cpu_lib PUBLIC common)