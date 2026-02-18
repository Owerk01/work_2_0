add_library(helpers 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vars.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vars.h)

target_include_directories(helpers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(helpers PRIVATE Qt6::Widgets Qt6::Gui Qt6::Core)