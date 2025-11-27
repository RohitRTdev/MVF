#version 460 core

out vec4 frag_color;

layout (location = 0) in vec3 a_color;

uniform float uAlpha;

void main() {
    frag_color = vec4(a_color, uAlpha);
}
