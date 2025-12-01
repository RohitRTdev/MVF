#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 tex_coord;

uniform mat4 uMVP;
out vec3 atex_coord;

void main(){
    gl_Position = uMVP * vec4(position, 1.0);
    atex_coord = tex_coord;
}
