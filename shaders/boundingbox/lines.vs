#version 460 core

layout(location = 0) in vec3 position;

uniform mat4 uMVP;
uniform float uZoom;

void main()
{
    vec4 pos = uMVP * vec4(Position, 1.0);
    pos.xyz = pos.xyz * uZoom;
    gl_Position = pos;
}

