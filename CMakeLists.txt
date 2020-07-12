# This file is based on starter project from https://github.com/lefticus/cpp_starter_project
cmake_minimum_required(VERSION 3.16)

# Set the project name to your project name
project(lbc CXX)
include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)

# Link this 'library' to set the c++ standard / compile-time options requested
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

# This sets a global PCH parameter, each project will build its own PCH, which is a good idea if any #define's change
#
# consider breaking this out per project as necessary
target_precompile_headers(project_options INTERFACE src/pch.h)

# LLVM
find_package(LLVM REQUIRED CONFIG)
llvm_map_components_to_libnames(
    llvm_libs
    core
    support
    bitwriter)

# Project specific compiler options
if(MSVC)

elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    target_compile_options(project_options INTERFACE -fno-exceptions -fno-rtti)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(project_options INTERFACE -fno-exceptions -fno-rtti)
else()
    message(AUTHOR_WARNING "No compiler options set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
endif()

# Source Code
add_subdirectory(src)
