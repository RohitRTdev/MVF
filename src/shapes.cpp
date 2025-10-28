#include <cstring>
#include "shapes.h"

constexpr uint8_t SEGMENTS = 20;
const uint8_t GLYPH_SAMPLING = 1;

namespace MVF {
    ArrowMesh::ArrowMesh(float rad_cyl, float rad_cone, float len_cyl, float len_cone) {
        // Generate the cylinder
        for (int i = 0; i < SEGMENTS; i++) {
            float theta = i * (2 * std::numbers::pi / SEGMENTS);
            float x = std::cos(theta) * rad_cyl;
            float y = std::sin(theta) * rad_cyl;

            vertices.push_back(ArrowVertex{.x = x, .y = y, .z = 0, .u = 0, .v = 0, .w = 0});
            vertices.push_back(ArrowVertex{.x = x, .y = y, .z = len_cyl, .u = 0, .v = 0, .w = 0});

            indices.push_back(2 * i);
            indices.push_back(2 * i + 1);
            indices.push_back((2 * i + 3) % (2 * SEGMENTS));
            indices.push_back(2 * i);
            indices.push_back((2 * i + 3) % (2 * SEGMENTS));
            indices.push_back((2 * i + 2) % (2 * SEGMENTS));
        }
        
        // Generate the cone
        auto cone_tip_idx = vertices.size();
        vertices.push_back(ArrowVertex{.x = 0, .y = 0, .z = len_cyl + len_cone, .u = 0, .v = 0, .w = 0});
        for (int i = 0; i < SEGMENTS; i++) {
            float theta = i * (2 * std::numbers::pi / SEGMENTS);
            float x = std::cos(theta) * rad_cone;
            float y = std::sin(theta) * rad_cone;

            vertices.push_back(ArrowVertex{.x = x, .y = y, .z = len_cyl, .u = 0, .v = 0, .w = 0});

            if (i == SEGMENTS - 1) {
                indices.push_back(cone_tip_idx + i + 1);
                indices.push_back(cone_tip_idx);
                indices.push_back(cone_tip_idx + 1);
            }
            else {
                indices.push_back(cone_tip_idx + i + 1);
                indices.push_back(cone_tip_idx);
                indices.push_back(cone_tip_idx + i + 2);
            }
        }

        compute_normals();
    } 

    void ArrowMesh::compute_normals() {
        int num_triangles = indices.size() / 3;

        // Apply weighted area normal computation
        // Normalization is done by the shaders
        for (int p = 0; p < num_triangles; p++) {
            auto& v0 = vertices[indices[p * 3]];
            auto& v1 = vertices[indices[p * 3 + 1]];
            auto& v2 = vertices[indices[p * 3 + 2]];

            Vector3f vec1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
            Vector3f vec2(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

            Vector3f normal = vec2.cross(vec1);

            for (int i = 0; i < 3; i++) {
                vertices[indices[p * 3 + i]].u += normal.x;
                vertices[indices[p * 3 + i]].v += normal.y;
                vertices[indices[p * 3 + i]].w += normal.z;
            }
        }

    }
        
    BoundingBox::BoundingBox(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax) {
        vertices[0] = Vertex{xmin, ymin, zmin};
        vertices[1] = Vertex{xmin, ymax, zmin};
        vertices[2] = Vertex{xmin, ymax, zmax};
        vertices[3] = Vertex{xmin, ymin, zmax};
        vertices[4] = Vertex{xmax, ymin, zmin};
        vertices[5] = Vertex{xmax, ymax, zmin};
        vertices[6] = Vertex{xmax, ymax, zmax};
        vertices[7] = Vertex{xmax, ymin, zmax};

        indices = { 
            0, 1, 1, 2, 2, 3, 3, 0,
            0, 4, 1, 5, 2, 6, 3, 7,
            4, 5, 5, 6, 6, 7, 7, 4
        };
    }

    GlyphMesh::GlyphMesh(VolumeData* model) {
        size_t sample_point = 0;
        float max_wgt = 0;

        auto wgt_fn = [] (float u, float v, float w) {
            return u*u + v*v + w*w;
        };

        for (int i = 0; i < model->nz; i++) {
            auto z = model->origin.z + i * model->spacing.z;
            for (int j = 0; j < model->ny; j++) {
                auto y = model->origin.y + j * model->spacing.y;
                for (int k = 0; k < model->nx; k++) {
                    if (sample_point % GLYPH_SAMPLING != 0) {
                        sample_point += 1;
                        continue;
                    }
                    auto x = model->origin.x + k * model->spacing.x;
                    auto u_pt = std::get<0>(model->scalars["U"])[i * model->ny * model->nx + j * model->nx + k];  
                    auto v_pt = std::get<0>(model->scalars["V"])[i * model->ny * model->nx + j * model->nx + k];  
                    auto w_pt = std::get<0>(model->scalars["W"])[i * model->ny * model->nx + j * model->nx + k];  
                    points.push_back(GlyphInstance{Vector3f(x, y, z), Vector3f(u_pt, v_pt, w_pt)});

                    max_wgt = std::max(max_wgt, wgt_fn(u_pt, v_pt, w_pt));
                    sample_point += 1;
                }
            }
        }
       
        // Second pass, apply the scale
        size_t pt = 0;
        sample_point = 0;
        for (int i = 0; i < model->nz; i++) {
            for (int j = 0; j < model->ny; j++) {
                for (int k = 0; k < model->nx; k++) {
                    if (sample_point % GLYPH_SAMPLING != 0) {
                        sample_point += 1;
                        continue;
                    }
                    points[pt].scale = wgt_fn(points[pt].direction.x, points[pt].direction.y, points[pt].direction.z) / max_wgt;

                    sample_point += 1;
                    pt++;
                }
            }
        }
    }
}

