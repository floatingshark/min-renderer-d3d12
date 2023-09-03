cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

#set(CMAKE_C_COMPILER c:/LLVM/bin/clang.exe)
#set(CMAKE_CXX_COMPILER c:/LLVM/bin/clang.exe)

project(albedos)

add_definitions(-Wall -Wextra -g -std=c++17)

include_directories(./Include)

link_directories(./Library/GLFW/lib-static-ucrt)
link_directories(./Library/GLFW/lib-vc2022)

FILE(GLOB main_sources Source/*.cpp)
FILE(GLOB imgui_sources Include/imgui/*.cpp)

add_executable(main ${main_sources} ${imgui_sources})

target_link_libraries(main glfw3 glfw3dll d3d12 dxgi d3dcompiler)