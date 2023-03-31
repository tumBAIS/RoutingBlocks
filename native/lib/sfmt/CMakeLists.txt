cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

project(SFMT VERSION 1.0 LANGUAGES CXX)

file(GLOB_RECURSE sfmt_headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/sfmt/*.h")
file(GLOB_RECURSE sfmt_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/sfmt/*.cpp")

add_library(${PROJECT_NAME} ${sfmt_headers} ${sfmt_sources})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:-O0;-g3>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:-O3>")

set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDUAL_OPTIMIZATION $<$<CONFIG:RELEASE>:TRUE>)
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

