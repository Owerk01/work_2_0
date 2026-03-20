add_library(logic 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/line_drawing_algs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/curved_lines_draw_algs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/funcs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/obj_3D.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/polygon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/polygon_fill.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/polygonalisation.cpp
)
target_include_directories(logic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)