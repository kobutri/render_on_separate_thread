cmake_minimum_required(VERSION 3.17)
project(render_on_separate_thread)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external/glad)

add_executable(render_on_separate_thread main.cpp)
target_link_libraries(render_on_separate_thread glad)