# ==============================================================================
# 4. EXECUTABLES (MONITOR SERVICE & MONITOR CLIENT)
# ==============================================================================

# ------------------------------------------------------------------------------
# Service Process (Runs in the background to collect data)
# ------------------------------------------------------------------------------
add_executable(monitor_service
    monitor_service.cpp
)

target_link_libraries(monitor_service PRIVATE
    monitor_lib
    signal_lib
    ipc_lib
    app_lib
)

# ------------------------------------------------------------------------------
# Client Process (Runs the display interface)
# ------------------------------------------------------------------------------
add_executable(monitor_client
    monitor_client.cpp
)

target_link_libraries(monitor_client PRIVATE
    ui_lib
    signal_lib
    ipc_lib
    app_lib
)

# ------------------------------------------------------------------------------
# Qt GUI Client App (Phase 5 - Task 1)
# ------------------------------------------------------------------------------
add_executable(monitor_gui
    monitor_gui.cpp
)

target_link_libraries(monitor_gui PRIVATE
    Qt6::Widgets
    gui_lib
    ipc_lib
    ui_lib
    signal_lib
    app_lib
)
