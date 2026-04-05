cmake_minimum_required(VERSION 3.16)
project(GraphicalEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets Gui Core REQUIRED)

add_executable(editoR ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/metadata)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/logic)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/model)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/view)

target_link_libraries(editoR PRIVATE Qt6::Widgets Qt6::Gui Qt6::Core)
target_link_libraries(editoR PRIVATE metadata logic model view)

if(ENABLE_SANITIZERS)
    message(STATUS "Sanitizers enabled!")
    target_compile_options(editoR PRIVATE 
        -fsanitize=address 
        -fno-omit-frame-pointer 
        -g 
        -O1
    )
    target_link_options(editoR PRIVATE 
        -fsanitize=address
    )
endif()