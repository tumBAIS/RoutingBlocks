cmake_minimum_required(VERSION 3.15)
include(CheckIPOSupported)

project(cvrp_vrpis_extension LANGUAGES CXX VERSION 1.0)

# To force building for a specific python version, set Python_ROOT_DIR to the respective path.
# This can be a venv as well.
# See the FIND_PACKAGE python cmake documentation for details.
FIND_PACKAGE(Python COMPONENTS Interpreter Development REQUIRED)

message(STATUS "Found python binary: ${Python_EXECUTABLE} (Version ${Python_VERSION})")

set(PYBIND11_PYTHON_VERSION ${Python_VERSION})
set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})
set(PYTHON_LIBS ${Python_LIBS})
FIND_PACKAGE(pybind11 CONFIG REQUIRED)

file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

set(VRPIS_INCLUDE_BASE "${Python_SITELIB}/include")

if (NOT IS_DIRECTORY ${VRPIS_INCLUDE_BASE}/vrpis)
    message(FATAL_ERROR "Could not find vrpis interface header directory. Please install the vrpis python package first.")
else ()
    message(STATUS "Found vrpis include directory: ${VRPIS_INCLUDE_BASE}")
endif ()

pybind11_add_module(${PROJECT_NAME} ${sources})

target_include_directories(${PROJECT_NAME} PUBLIC ${VRPIS_INCLUDE_BASE})
target_compile_definitions(${PROJECT_NAME} PUBLIC "VRPIS_EXT_MODULE_NAME=${PROJECT_NAME}")
target_compile_definitions(${PROJECT_NAME} PUBLIC "VRPIS_EXT_MODULE_VERSION=${PROJECT_VERSION}")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:-O0;-g3>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:-O3>")

check_ipo_supported(RESULT ${PROJECT_NAME}_HAS_LTO)
if (${${PROJECT_NAME}_HAS_LTO})
    message(STATUS "LTO is supported")
    set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDUAL_OPTIMIZATION TRUE)
endif ()

#
# Set the compiler standard
#
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)