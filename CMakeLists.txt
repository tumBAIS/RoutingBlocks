cmake_minimum_required(VERSION 3.15)

project(_${SKBUILD_PROJECT_NAME} VERSION ${SKBUILD_PROJECT_VERSION})
string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)

# Build the native library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/native)
# Build the Python extension
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/bindings)

install(TARGETS ${PROJECT_NAME} DESTINATION ${SKBUILD_PROJECT_NAME} LIBRARY DESTINATION ${SKBUILD_PROJECT_NAME})