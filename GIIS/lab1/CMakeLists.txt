cmake_minimum_required(VERSION 3.16)
project(GraphicalEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets Gui Core REQUIRED)

add_executable(editoR ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/helpers)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/logic)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/model)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/view)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/controller)

target_link_libraries(editoR PRIVATE Qt6::Widgets Qt6::Gui Qt6::Core)
target_link_libraries(editoR PRIVATE helpers logic model view contr)