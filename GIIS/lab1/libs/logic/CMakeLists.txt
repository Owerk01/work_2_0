add_library(logic 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/line_drawing_algs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/curved_lines_draw_algs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/funcs.cpp
)
target_include_directories(logic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)