#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 camera;
uniform mat4 transform;

void main()
{
    gl_Position = camera * transform * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}