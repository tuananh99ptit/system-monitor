# ==============================================================================
# CREATE STATIC LIBRARY: libconcurrency_lib.a
# ==============================================================================
find_package(Threads REQUIRED)

add_library(
    concurrency_lib STATIC
    producer.cpp
    consumer.cpp
)

target_include_directories(
    concurrency_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    concurrency_lib PUBLIC
    cpu_lib
    common
    Threads::Threads
)