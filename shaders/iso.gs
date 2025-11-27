#version 460 core

const int edge_vertex_indices[24] = {
    0, 1, 1, 3, 3, 2, 2, 0, 4, 5, 5, 7,
    7, 6, 6, 4, 0, 4, 1, 5, 3, 7, 2, 6
};

layout(std430, binding = 0) buffer Tri_Table {
    int tri_table[];
};

layout(std430, binding = 1) buffer Edge_Table {
    int edge_table[];
};

layout (points) in;
layout (triangle_strip, max_vertices = 12) out;

uniform mat4 uMVP;
uniform mat4 uM;
uniform float uIsoValue;
uniform vec3 uOrigin;
uniform vec3 uLimits;
uniform vec3 uSpacing;
uniform vec3 uSteps;
uniform bool uApplyColor;
uniform sampler3D volume_tex;
uniform sampler3D color_tex;

layout (location = 0) in vec3 voxel_pos[];

layout (location = 0) out vec3 a_normal;
layout (location = 1) out vec3 a_frag_pos;
layout (location = 2) out vec3 a_color;

void main() {
    vec3 position = voxel_pos[0];
    vec3 neighbours[8] = {
        position, vec3(position.x + uSteps.x, position.y, position.z),
        vec3(position.x, position.y + uSteps.y, position.z), vec3(position.x + uSteps.x, position.y + uSteps.y, position.z),
        vec3(position.x, position.y, position.z + uSteps.z), vec3(position.x + uSteps.x, position.y, position.z + uSteps.z),
        vec3(position.x, position.y + uSteps.y, position.z + uSteps.z), vec3(position.x + uSteps.x, position.y + uSteps.y, position.z + uSteps.z)
    };
    
    float scalars[8];
    int bitmask = 0;
    for (int idx = 0; idx < 8; idx++) {
        float val = texture(volume_tex, neighbours[idx]).r;
        bitmask |= (((val <= uIsoValue) ? 1 : 0) << idx);
        scalars[idx] = val;
    }

    vec3 iso_points[12];
    for (int edge = 0; edge < 12; edge++) {
        // We need to find isopoint for this edge
        if ((edge_table[bitmask] & (1 << edge)) != 0) {
            int v0_idx = edge_vertex_indices[edge * 2];
            int v1_idx = edge_vertex_indices[edge * 2 + 1];
            
            float denom = scalars[v1_idx] - scalars[v0_idx];
            float t = (abs(denom) < 1e-6) ? 0.5 : (uIsoValue - scalars[v0_idx]) / denom;
            vec3 iso_point = neighbours[v0_idx] + (neighbours[v1_idx] - neighbours[v0_idx]) * t;

            // Now we compute the point in object space
            iso_point = uOrigin + iso_point * uSpacing * uLimits; 
            iso_points[edge] = iso_point;
        }
    }
    
    int cur_idx = 0;
    int base = bitmask * 16;
    while (cur_idx <= 13 && tri_table[base + cur_idx] != -1) {
        vec3 v[3];
        for (int j = 0; j < 3; j++) {
            v[j] = iso_points[tri_table[base + cur_idx + j]];    
        }
        
        vec3 d1 = v[1] - v[0];
        vec3 d2 = v[2] - v[0];
        vec3 nvec = cross(d2, d1);
        float nlen = length(nvec);
        
        // Skip degenerate triangle
        if (nlen < 1e-6) {
            cur_idx += 3;
            continue;
        }
        
        vec3 normal = normalize(mat3(uM) * nvec);
        for (int j = 0; j < 3; j++) {
            gl_Position = uMVP * vec4(v[j], 1.0);
            a_normal = normal;
            a_frag_pos = (uM * vec4(v[j], 1.0)).xyz;
            if (uApplyColor) {
                a_color = texture(color_tex, position).rgb;
            }
            else {
                a_color = vec3(1.0, 0.0, 0.0);
            }
            EmitVertex();
        }

        EndPrimitive(); 
        cur_idx += 3;
    }
}

