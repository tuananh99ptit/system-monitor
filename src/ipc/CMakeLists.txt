# ==============================================================================
# CREATE STATIC LIBRARY: libipc_lib.a
# ==============================================================================

add_library(
    ipc_lib STATIC
    unix_socket_server.cpp
)

target_include_directories(
    ipc_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)