cmake_minimum_required(VERSION 3.14)

project(VRPIS VERSION 0.1 LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/tools.cmake)

option(${PROJECT_NAME}_BUILD_TESTS on)
option(${PROJECT_NAME}_ENABLE_ASAN off)
option(${PROJECT_NAME}_BUILD_NATIVE off)
option(${PROJECT_NAME}_ENABLE_LTO on)

CPMAddPackage(
        NAME fmt
        GIT_TAG 8.1.1
        GITHUB_REPOSITORY fmtlib/fmt
        OPTIONS "FMT_INSTALL YES" # create an installable target
)
set_target_properties(fmt PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_subdirectory(lib)

file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/vrpis/*.h")
file(GLOB_RECURSE public_headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/vrpis/interfaces/*.h")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(${PROJECT_NAME} ${headers} ${sources})
target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt)
target_link_libraries(${PROJECT_NAME} PUBLIC SFMT)
target_link_libraries(${PROJECT_NAME} PUBLIC DYNAMIC_BITSET)
target_link_libraries(${PROJECT_NAME} PUBLIC SMALL_VECTOR)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20 POSITION_INDEPENDENT_CODE ON)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${public_headers}")

target_include_directories(
        ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

if ( CMAKE_COMPILER_IS_GNUCC )
    target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic)
endif()
if ( MSVC )
    target_compile_options(main PRIVATE /W4)
endif()

target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:-O0;-g3>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:-O3;>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:PROFILE>:-O2;-pg;-fno-omit-frame-pointer;-fno-optimize-sibling-calls;>")

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

INSTALL(TARGETS ${PROJECT_NAME}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vrpis
        )
