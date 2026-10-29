# ==============================================================================
# CREATE STATIC LIBRARY: libsignal_lib.a
# ==============================================================================
find_package(Threads REQUIRED)

add_library(
    signal_lib STATIC
    signal_handler.cpp
)

# Declare header directories directly associated with the 'signal_lib' target
target_include_directories(
    signal_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies
target_link_libraries(signal_lib PUBLIC Threads::Threads)