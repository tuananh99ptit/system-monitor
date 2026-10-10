# ==============================================================================
# 1. CREATE STATIC LIBRARY: libcommon.a
# ==============================================================================
add_library(
    common STATIC
    logger/logger.cpp
    file_reader/file_reader.cpp
    utils/utils.cpp
)

# Declare header directories directly associated with the 'common' target
target_include_directories(
    common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/logger
    ${CMAKE_CURRENT_SOURCE_DIR}/file_reader
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/config
)