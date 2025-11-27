#version 460 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec3 color;

layout (location = 0) out vec3 a_color;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    gl_PointSize = 4.0;
    a_color = color;
}
