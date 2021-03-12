cmake_minimum_required(VERSION 3.17)
project(glad)

set(CMAKE_CXX_STANDARD 20)

add_library(glad glad.c glad.h khrplatform.h)