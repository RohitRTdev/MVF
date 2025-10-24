#version 460 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 texCoord;
layout(location = 2) in vec3 color;

uniform mat4 uMVP;
uniform mat4 uM;
uniform float uZoom;

layout(location = 0) out vec3 aTexCoord;
layout(location = 1) out vec3 aNormal;
layout(location = 2) out vec3 aFragPos;
layout(location = 3) out vec3 aColor;

void main()
{
    aTexCoord = texCoord;
    aNormal = (uM * vec4(texCoord, 0.0)).xyz; 
    aFragPos = (uM * vec4(Position, 1.0)).xyz; 
    aColor = color;
    vec4 pos = uMVP * vec4(Position, 1.0);
    pos.xyz = pos.xyz * uZoom;
    gl_Position = pos;
}
