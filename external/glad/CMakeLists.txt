cmake_minimum_required(VERSION 3.17)
project(glad)

set(CMAKE_CXX_STANDARD 20)
add_library(glad src/glad.c)
target_include_directories(glad PUBLIC include)