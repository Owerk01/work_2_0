add_library(metadata 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vars.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/data_definitions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc.cpp)

target_include_directories(metadata PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)