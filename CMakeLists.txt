cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(Arabesque)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
add_definitions(-Wall )

add_executable(main ./Source/main.cpp)