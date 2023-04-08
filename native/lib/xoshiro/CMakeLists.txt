cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(XOSHIRO VERSION 1.0 LANGUAGES CXX)


add_library(XOSHIRO INTERFACE)
target_include_directories(XOSHIRO INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})


