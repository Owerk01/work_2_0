add_library(contr 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/toolbarcontr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/toolbarcontr.h
    
)
target_include_directories(contr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(contr PRIVATE Qt6::Widgets Qt6::Gui Qt6::Core)
target_link_libraries(contr PRIVATE helpers view logic)
