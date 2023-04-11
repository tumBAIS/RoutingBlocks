cmake_minimum_required(VERSION 3.15)
include(CheckIPOSupported)
# To force building for a specific python version, set Python_ROOT_DIR to the respective path.
# This can be a venv as well.
# See the FIND_PACKAGE python cmake documentation for details.
FIND_PACKAGE(Python COMPONENTS Interpreter Development.Module REQUIRED)

message(STATUS "Found python binary: ${Python_EXECUTABLE} (Version ${Python_VERSION})")

set(PYBIND11_PYTHON_VERSION ${Python_VERSION})
set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})
set(PYTHON_LIBS ${Python_LIBS})
FIND_PACKAGE(pybind11 CONFIG REQUIRED)

option(${PROJECT_NAME_UPPER}_ENABLE_STUB_GEN "Generate python stub files for the bindings generated." ON)

file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/routingblocks_bindings/*.h")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

pybind11_add_module(${PROJECT_NAME} ${sources})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(${PROJECT_NAME} PUBLIC "routingblocks_MODULE_NAME=${PROJECT_NAME}")
target_compile_definitions(${PROJECT_NAME} PUBLIC "routingblocks_VERSION=${PROJECT_VERSION}")

# Configure optimization flags
if (CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:-O0;-g3>")
    target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:-O3;>")
    target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:PROFILE>:-O2;-pg;-fno-omit-frame-pointer;-fno-optimize-sibling-calls;>")
endif ()
if (MSVC)
    target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:/Od>")
    target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:/O2y>")
endif ()

# Link to main routing blocks library
target_link_libraries(${PROJECT_NAME} PUBLIC routingblocks)

check_ipo_supported(RESULT ${PROJECT_NAME}_HAS_LTO)
if (${${PROJECT_NAME}_HAS_LTO})
    message(STATUS "LTO is supported")
    set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDUAL_OPTIMIZATION TRUE)
endif ()

if (${PROJECT_NAME_UPPER}_ENABLE_STUB_GEN)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/stubgen.py -m ${PROJECT_NAME} --out ${CMAKE_CURRENT_BINARY_DIR} WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}> VERBATIM)
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pyi DESTINATION routingblocks)
endif ()

#
# Set the compiler standard
#
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

INSTALL(TARGETS routingblocks DESTINATION routingblocks)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/routingblocks_bindings/binding_helpers.hpp DESTINATION routingblocks/include/routingblocks/)