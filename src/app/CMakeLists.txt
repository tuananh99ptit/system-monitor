# ==============================================================================
# 1. CREATE STATIC LIBRARY: libapp_lib.a
# ==============================================================================
add_library(
    app_lib STATIC
    app.cpp
)

# Declare header directories directly associated with the 'app_lib' target
target_include_directories(
    app_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link all submodules and common library
target_link_libraries(
    app_lib PUBLIC
    cpu_lib
    memory_lib
    disk_lib
    process_lib
    common
)