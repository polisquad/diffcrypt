cmake_minimum_required(VERSION 3.12)

# Project setup ---------------------------------
project(diffcrypt)

## Platform setup
include(cmake/platform.cmake)

## Build type config
set(CMAKE_CXX_FLAGS_DEBUG			"-DSGL_BUILD_DEBUG -DBUILD_DEBUG=1 -ggdb -O0")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO	"-DSGL_BUILD_DEVELOPMENT -DBUILD_DEVELOPMENT=1 -ggdb -O3")
set(CMAKE_CXX_FLAGS_RELEASE			"-DSGL_BUILD_RELEASE -DBUILD_RELEASE=1 -Ofast")

# Compiler setup --------------------------------
## CXX preferences
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED false)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAG} -mavx2 -pthread -fopenmp") # Define compiler flags

# Code setup ------------------------------------
## Source code
add_subdirectory(src)