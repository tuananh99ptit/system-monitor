# ==============================================================================
# EXECUTABLES (MONITOR SERVICE & MONITOR CLIENT)
# ==============================================================================

# 1. Service Process (Runs in the background to collect data)
add_executable(monitor_service monitor_service.cpp)
target_link_libraries(
    monitor_service PRIVATE
    monitor_lib
    signal_lib
)

# 2. Client Process (Runs the display interface)
add_executable(monitor_client monitor_client.cpp)
target_link_libraries(
    monitor_client PRIVATE
    ui_lib
    signal_lib
)
