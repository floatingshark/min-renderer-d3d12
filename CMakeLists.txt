cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(Arabesque)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
add_definitions(-Wall -g -std=c++11)

include_directories(./Include)

link_directories(./Library/GLFW)

add_executable(main ./Source/main.cpp)

target_link_libraries(main glfw3 glfw3dll d3d12 dxgi d3dcompiler)