# ==============================================================================
# CREATE STATIC LIBRARY: libui_lib.a
# ==============================================================================
add_library(
    ui_lib STATIC
    console_renderer.cpp
)

# Declare header directories directly associated with the 'ui_lib' target
target_include_directories(
    ui_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies (cần monitor_data.h)
target_link_libraries(ui_lib PUBLIC monitor_lib)