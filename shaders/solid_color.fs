#version 460 core

out vec4 frag_color;

uniform vec4 uColor;

void main() {
    frag_color = vec4(uColor);
}