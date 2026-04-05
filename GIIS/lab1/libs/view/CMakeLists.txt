add_library(view 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/window.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/canvas.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/canvas.h
)
target_include_directories(view PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(view PRIVATE Qt6::Widgets Qt6::Gui Qt6::Core)
target_link_libraries(view PRIVATE metadata model logic)