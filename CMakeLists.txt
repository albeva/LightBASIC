# This file is based on starter project from https://github.com/lefticus/cpp_starter_project
cmake_minimum_required(VERSION 3.16)

# Set the project name to your project name
project(lbc CXX)
include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)
include(cmake/llvm.cmake)

# Link this 'library' to set the c++ standard / compileSource-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
    if(ENABLE_BUILD_WITH_TIME_TRACE)
        add_compile_definitions(project_options INTERFACE -ftime-trace)
    endif()
endif()

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# allow for static analysis options
include(cmake/StaticAnalyzers.cmake)

# Project specific compiler options
if(MSVC)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
    endif()
    target_compile_options(project_options INTERFACE "/Zc:__cplusplus")
elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    target_compile_options(project_options INTERFACE -fno-exceptions -fno-rtti)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(project_options INTERFACE -fno-exceptions -fno-rtti)
else()
    message(AUTHOR_WARNING "No compiler options set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
endif()

# Source Code
add_subdirectory(src)

# Copy compiled binary
set_target_properties(lbc PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# Testing
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    enable_testing()
    add_subdirectory(tests)
endif()
