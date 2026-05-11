cmake_minimum_required(VERSION 3.20)

project(muduo_lib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 是否构建例子
option(M_MUDUO_BUILD_EXAMPLES "Build examples" ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_subdirectory(muduo)

if(M_MUDUO_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()
