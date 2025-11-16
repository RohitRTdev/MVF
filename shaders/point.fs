#version 460 core

layout (location = 0) in vec2 uv;
uniform vec4 uColor;

out vec4 frag_color;

void main()
{
    float d = length(uv);
    float radius = 1.0;
    float aa = 0.01;

    float alpha = smoothstep(radius, radius - aa, d);

    frag_color = vec4(uColor.rgb, uColor.a * alpha);
}
