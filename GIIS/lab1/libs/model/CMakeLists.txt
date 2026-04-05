add_library(model 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/debugger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/data_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/data_handler.h
)
target_include_directories(model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(model PRIVATE Qt6::Widgets Qt6::Gui Qt6::Core)
target_link_libraries(model PRIVATE metadata view logic)
