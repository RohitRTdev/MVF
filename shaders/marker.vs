#version 460 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 inst_position;
layout (location = 2) in vec3 color;

layout(location = 0) out vec3 a_color;

void main() {
    gl_Position = vec4(position + inst_position, 0, 1.0);
    a_color = color;
}