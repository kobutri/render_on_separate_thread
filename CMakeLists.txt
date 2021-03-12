cmake_minimum_required(VERSION 3.17)
project(render_on_separate_thread)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(external/glad)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

add_subdirectory(external/glm-0.9.9.8)


add_executable(embedfile utils/embedfile.cpp)

add_custom_command(
        OUTPUT vertex_shader_source.c
        COMMAND embedfile vertex_shader_source ${PROJECT_SOURCE_DIR}/shaders/shader.vert
        DEPENDS shaders/shader.vert
)

add_custom_command(
        OUTPUT fragment_shader_source.c
        COMMAND embedfile fragment_shader_source ${PROJECT_SOURCE_DIR}/shaders/shader.frag
        DEPENDS shaders/shader.frag
)

add_executable(render_on_separate_thread main.cpp fragment_shader_source.c vertex_shader_source.c)

target_link_libraries(render_on_separate_thread glad glfw glm)
