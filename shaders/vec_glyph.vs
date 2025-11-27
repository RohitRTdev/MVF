#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 inst_position;
layout(location = 3) in vec3 inst_direction;
layout(location = 4) in float scale_factor;

uniform mat4 uMVP;
uniform mat4 uM;

layout(location = 0) out vec3 a_normal;
layout(location = 1) out vec3 a_frag_pos;
layout(location = 2) out vec3 a_color;

void main() {
    vec3 Z = normalize(inst_direction);
    vec3 up = vec3(0.0, 1.0, 0.0);

    // Handle case when dir is almost parallel to up
    if (abs(dot(Z, up)) > 0.999) {
        up = vec3(1.0, 0.0, 0.0);
    }

    vec3 X = normalize(cross(up, Z));
    vec3 Y = cross(Z, X);

    mat4 rot = mat4(
        vec4(X, 0.0),
        vec4(Y, 0.0),
        vec4(Z, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );

    mat4 trans = mat4(1.0);
    trans[3].xyz = inst_position;

    mat4 scale = mat4(1.0);
    scale[0][0] = scale_factor;
    scale[1][1] = scale_factor;
    scale[2][2] = scale_factor;

    mat4 model_inst = trans * rot * scale;

    vec4 world_pos = uM * model_inst * vec4(position, 1.0);
    a_frag_pos = world_pos.xyz;
    a_normal = mat3(uM * model_inst) * normal; 

    gl_Position = uMVP * model_inst * vec4(position, 1.0);
    a_color = vec3(1.0, 0.0, 0.0);
}
