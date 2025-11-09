#version 460 core

layout (location = 0) in vec3 position;

layout (location = 0) out vec3 voxel_pos;
void main() {
    gl_Position = vec4(position, 1.0);
}
