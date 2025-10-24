#version 460 core

layout (location = 0) in vec3 Position;

layout(location = 0) out vec3 aVoxelPos;

void main() {
    aVoxelPos = Position;
}