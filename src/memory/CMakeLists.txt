# ==============================================================================
# 1. CREATE STATIC LIBRARY: libmemory_lib.a
# ==============================================================================
add_library(
    memory_lib STATIC
    memory_monitor.cpp
)

# Declare header directories directly associated with the 'memory_lib' target
target_include_directories(
    memory_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies
target_link_libraries(memory_lib PUBLIC common)