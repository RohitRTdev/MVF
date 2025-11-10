#version 460 core

layout(location = 0) in vec3 position; // unit quad in XY in range [-1,1]

uniform mat4 uMVP;
uniform vec3 uBBoxMin;
uniform vec3 uBBoxMax;
uniform int uSlices;

out vec3 vTexCoord;

void main() {
    // Map XY from [-1,1] to [0,1]
    vec2 uv = position.xy * 0.5 + 0.5;
    float t = (uSlices > 1) ? float(gl_InstanceID) / float(uSlices - 1) : 0.0;

    // Build model-space position on the current slice plane (Z varies with instance)
    vec3 pos;
    pos.x = mix(uBBoxMin.x, uBBoxMax.x, uv.x);
    pos.y = mix(uBBoxMin.y, uBBoxMax.y, uv.y);
    pos.z = mix(uBBoxMin.z, uBBoxMax.z, t);

    // Texture coordinates normalized to [0,1]^3 in model space
    vTexCoord = (pos - uBBoxMin) / (uBBoxMax - uBBoxMin);

    gl_Position = uMVP * vec4(pos, 1.0);
}
