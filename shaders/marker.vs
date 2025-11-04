#version 460 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 inst_position;

void main() {
    gl_Position = vec4(position + inst_position, 0, 1.0);
}