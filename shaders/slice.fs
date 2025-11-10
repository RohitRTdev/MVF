#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTex; // R8 normalized slice values in [0,1]

// Simple multi-stop blue->cyan->green->yellow->red colormap
vec3 colormap(float t){
    t = clamp(t, 0.0, 1.0);
    if (t < 0.25) { // blue -> cyan
        float u = t / 0.25;
        return mix(vec3(0.0,0.0,1.0), vec3(0.0,1.0,1.0), u);
    } else if (t < 0.50) { // cyan -> green
        float u = (t-0.25)/0.25;
        return mix(vec3(0.0,1.0,1.0), vec3(0.0,1.0,0.0), u);
    } else if (t < 0.75) { // green -> yellow
        float u = (t-0.50)/0.25;
        return mix(vec3(0.0,1.0,0.0), vec3(1.0,1.0,0.0), u);
    } else { // yellow -> red
        float u = (t-0.75)/0.25;
        return mix(vec3(1.0,1.0,0.0), vec3(1.0,0.0,0.0), u);
    }
}

void main(){
    float v = texture(uTex, vUV).r; // already normalized 0..1
    vec3 c = colormap(v);
    FragColor = vec4(c, 1.0);
}
