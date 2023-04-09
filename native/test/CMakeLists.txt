cmake_minimum_required(VERSION 3.15)
Include(FetchContent)

#
# Project details
#

project(
        ${CMAKE_PROJECT_NAME}Tests
        LANGUAGES CXX
)

message("Adding tests under ${CMAKE_PROJECT_NAME}Tests...")

#
# Set the sources for the unit tests and add the executable(s)
#

file(GLOB_RECURSE test_headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/routingblocks-tests/*.h")
file(GLOB_RECURSE test_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(${PROJECT_NAME} ${test_headers} ${test_sources})

#
# Set seed for reproducibility
#
target_compile_definitions(${PROJECT_NAME} PUBLIC -DROUTINGBLOCKS_RAND_SEED=2021)

#
# Set the compiler standard
#

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# Disable optimization
#target_compile_options(${CMAKE_PROJECT_NAME} BEFORE PUBLIC -O0 -g)

# Include headers
target_include_directories(${PROJECT_NAME} PUBLIC include/)

#
# Enable ASAN
#

if (${CMAKE_PROJECT_NAME}_ENABLE_ASAN)
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
    target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
endif ()

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.1.0
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        Catch2::Catch2WithMain
        ${CMAKE_PROJECT_NAME}
)

include(Catch)
catch_discover_tests(${PROJECT_NAME})

message("Finished adding unit tests for ${CMAKE_PROJECT_NAME}.")
