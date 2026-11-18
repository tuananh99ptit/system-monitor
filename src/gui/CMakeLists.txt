add_library(gui_lib STATIC
    main_window.cpp
    main_window.h
)

target_include_directories(gui_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(gui_lib PUBLIC
    Qt6::Widgets
)