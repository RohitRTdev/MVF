#version 460 core

in vec3 atex_coord;

uniform sampler3D slice_tex;

out vec4 frag_color;

vec3 color_map(float t) {
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
    frag_color = vec4(color_map(texture(slice_tex, atex_coord).r), 1.0);
}
