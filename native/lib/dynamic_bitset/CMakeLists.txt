cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(DYNAMIC_BITSET VERSION 1.0 LANGUAGES CXX)

add_library(DYNAMIC_BITSET INTERFACE)
target_include_directories(DYNAMIC_BITSET INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dynamic_bitset DESTINATION routingblocks/include/)