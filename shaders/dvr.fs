#version 460 core

in vec3 vTexCoord;
out vec4 frag_color;

uniform sampler3D uTex3D;
uniform float uAlphaScale;

vec3 jet(float t) {
    t = clamp(t, 0.0, 1.0);
    float r = clamp(1.5 - abs(4.0*t - 3.0), 0.0, 1.0);
    float g = clamp(1.5 - abs(4.0*t - 2.0), 0.0, 1.0);
    float b = clamp(1.5 - abs(4.0*t - 1.0), 0.0, 1.0);
    return vec3(r,g,b);
}

void main() {
    float val = texture(uTex3D, vTexCoord).r; // 0..1 normalized
    vec3 color = jet(val);
    float alpha = val * uAlphaScale;
    frag_color = vec4(color, alpha);
}
