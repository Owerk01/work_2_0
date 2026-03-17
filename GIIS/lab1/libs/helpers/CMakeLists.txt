add_library(helpers 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vars.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc.cpp)

target_include_directories(helpers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)