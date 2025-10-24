#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 texCoord;

uniform mat4 uMVP;
uniform float uZoom;

layout(location = 0) out vec3 aTexCoord;

void main()
{
    aTexCoord = texCoord;
    vec4 pos = uMVP * vec4(Position, 1.0);
    pos.xyz = pos.xyz * uZoom;
    gl_Position = pos;
}

