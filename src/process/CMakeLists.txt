# ==============================================================================
# 1. CREATE STATIC LIBRARY: libprocess_lib.a
# ==============================================================================
add_library(
    process_lib STATIC
    process_monitor.cpp
)

# Declare header directories directly associated with the 'process_lib' target
target_include_directories(
    process_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies
target_link_libraries(process_lib PUBLIC common)