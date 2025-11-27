#version 460 core

layout (location = 0) in vec3 a_normal;
layout (location = 1) in vec3 a_frag_pos;
layout (location = 2) in vec3 a_color;

out vec4 frag_color;

uniform vec3 uLightPos;
uniform vec3 uViewPos;

vec4 apply_phong_shading() {
    vec3 light_dir = normalize(uLightPos - a_frag_pos);
    vec3 view_dir = normalize(uViewPos - a_frag_pos);
    vec3 norm = normalize(a_normal);
    vec3 reflect_dir = reflect(-light_dir, norm);
    vec3 light_color = vec3(1.0, 1.0, 1.0);
    vec3 material_color = a_color;

    float kd = 0.6;
    float ks = 0.3;
    float ka = 0.1;

    vec3 specular = vec3(0.0);

    // This is to prevent any ghost highlight on back face
    if (dot(norm, light_dir) > 0.0) {
        specular = ks * pow(max(dot(reflect_dir, view_dir), 0.0), 3) * light_color;
    }

    vec3 diffuse = kd * max(dot(norm, light_dir), 0.0) * material_color * light_color;
    vec3 ambient = ka * 1.0 * light_color * material_color;

    return vec4(diffuse + specular + ambient, 1.0);
}

void main() {
    frag_color = apply_phong_shading();
}
