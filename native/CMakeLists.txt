cmake_minimum_required(VERSION 3.15)

project(routingblocks VERSION 0.1 LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/tools.cmake)

option(${PROJECT_NAME}_BUILD_TESTS on)
option(${PROJECT_NAME}_ENABLE_ASAN off)
option(${PROJECT_NAME}_BUILD_NATIVE off)
option(${PROJECT_NAME}_ENABLE_LTO on)

add_subdirectory(lib)

file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/routingblocks/*.h")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(${PROJECT_NAME} STATIC ${headers} ${sources})
target_link_libraries(${PROJECT_NAME} PUBLIC XOSHIRO)
target_link_libraries(${PROJECT_NAME} PUBLIC DYNAMIC_BITSET)
target_link_libraries(${PROJECT_NAME} PUBLIC SMALL_VECTOR)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20 POSITION_INDEPENDENT_CODE ON)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)

target_include_directories(
        ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

if (CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif ()
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif ()

if (CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:-O0;-g3>")
    target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:-O3;>")
    target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:PROFILE>:-O2;-pg;-fno-omit-frame-pointer;-fno-optimize-sibling-calls;>")
endif ()
if (MSVC)
    target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:/Od>")
    target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:/O2y>")
endif ()

if (${${PROJECT_NAME}_BUILD_NATIVE})
    target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:-march=native>")
endif ()

if (${${PROJECT_NAME}_ENABLE_LTO})
    CHECK_IPO_SUPPORTED(RESULT supported OUTPUT error)
    set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDUAL_OPTIMIZATION TRUE)
endif ()

if (${${PROJECT_NAME}_ENABLE_ASAN})
    target_compile_options(${PROJECT_NAME} PUBLIC "-fsanitize=address")
    target_link_options(${PROJECT_NAME} PUBLIC "-fsanitize=address")
endif ()

if (${${PROJECT_NAME}_BUILD_BINDINGS})
    add_subdirectory(bindings)
endif ()

if (${${PROJECT_NAME}_BUILD_TESTS})
    add_subdirectory(test)
endif ()

# Install native library
INSTALL(TARGETS ${PROJECT_NAME})
# Install include/ directory and libs
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/routingblocks DESTINATION routingblocks/include/)
INSTALL(TARGETS DYNAMIC_BITSET XOSHIRO SMALL_VECTOR)
