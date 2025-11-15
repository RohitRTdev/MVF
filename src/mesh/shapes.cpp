#include <cstring>
#include <algorithm>
#include <ranges>
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

    GlyphMesh::GlyphMesh(VolumeData* model, const std::string& field1, const std::string& field2, const std::string& field3) {
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
                    auto u_pt = std::get<0>(model->scalars[field1])[i * model->ny * model->nx + j * model->nx + k];  
                    auto v_pt = std::get<0>(model->scalars[field2])[i * model->ny * model->nx + j * model->nx + k];  
                    auto w_pt = std::get<0>(model->scalars[field3])[i * model->ny * model->nx + j * model->nx + k];  
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
    
    void Axis::push_line_quad(Vector2f& a, Vector2f& b, float thickness) {
        Vector2f dir = (b - a).normalize();
        Vector2f n(-dir.y, dir.x); 
        n = n * thickness * 0.5f;

        auto v0 = a - n;
        auto v1 = a + n;
        auto v2 = b + n;
        auto v3 = b - n;

        vertices.push_back(v0);
        vertices.push_back(v1);
        vertices.push_back(v2);
        vertices.push_back(v0);
        vertices.push_back(v2);
        vertices.push_back(v3);
    }

    void Axis::push_arrow(Vector2f& base, Vector2f& dir, float length, float width) {
        auto n = Vector2f(-dir.y, dir.x);
        auto tip = base + dir * length;
        auto v0 = tip;
        auto v1 = base + n * (width * 0.5f);
        auto v2 = base - n * (width * 0.5f);

        vertices.push_back(v0);
        vertices.push_back(v1);
        vertices.push_back(v2);
    }

    Axis::Axis(int tick_count, float axis_len, float axis_thickness, float tick_size, bool is_x_axis) {
        float half_l = axis_len * 0.5f;
        Vector2f ep1(-half_l, 0.0f);
        Vector2f ep2(half_l, 0.0f);

        if (!is_x_axis) {
            ep1 = Vector2f(0.0f, half_l);
            ep2 = Vector2f(0.0f, -half_l);
        }

        // Main axis line
        push_line_quad(ep1, ep2, axis_thickness);

        // Ticks
        for (int i = 0; i <= tick_count; i++) {
            float t = static_cast<float>(i) / tick_count;
            float pt = -half_l + t * axis_len;
            Vector2f a(pt, -tick_size * 0.5f);
            Vector2f b(pt, tick_size * 0.5f);
            if (!is_x_axis) {
                a = Vector2f(-tick_size * 0.5f, pt);
                b = Vector2f(tick_size * 0.5f, pt);
            }

            push_line_quad(a, b, axis_thickness * 0.5f);
        }

        // Arrows
        Vector2f dir_ep1(-1, 0);
        Vector2f dir_ep2(1, 0);

        if (!is_x_axis) {
            dir_ep1 = Vector2f(0, 1);
            dir_ep2 = Vector2f(0, -1);
        }

        float arrow_len = 0.07f;
        float arrow_width = 0.06f;

        push_arrow(ep1, dir_ep1, arrow_len, arrow_width);
        push_arrow(ep2, dir_ep2, arrow_len, arrow_width);
    }
        
    Vector2f PointMarker::rotate_line(Vector2f& pt, float angle) {
        float angle_rad = ToRadian(angle);
        float rotation_matrix[2][2] = {
        {std::cos(angle_rad), std::sin(angle_rad)}, 
        {-std::sin(angle_rad), std::cos(angle_rad)}
        };

        Vector2f pt_r;
        pt_r.x = pt.x * rotation_matrix[0][0] + pt.y * rotation_matrix[1][0];
        pt_r.y = pt.x * rotation_matrix[0][1] + pt.y * rotation_matrix[1][1];
    
        return pt_r; 
    }

    PointMarker::PointMarker(float length, float thickness) {
        Vector2f p1 = Vector2f(length / 2, thickness / 2);
        Vector2f p2 = Vector2f(length / 2, -thickness / 2);
        Vector2f p3 = Vector2f(-length / 2, -thickness / 2);
        Vector2f p4 = Vector2f(-length / 2, thickness / 2);
        
        std::array<Vector2f, 6> points = {p1, p2, p3, p3, p4, p1};
        for (auto angle : {45, -45}) {
            std::ranges::copy(
                points | std::views::transform([this, angle](auto& val){ return rotate_line(val, angle); }),
                std::back_inserter(vertices)
            );
        }
    }
    
    static void add_rect(std::vector<Vector2f>& vertices, float x, float y, float w, float h) {
        // Triangle 1
        vertices.push_back({x, y});
        vertices.push_back({x + w, y});
        vertices.push_back({x + w, y+h});

        // Triangle 2
        vertices.push_back({x, y});
        vertices.push_back({x + w, y + h});
        vertices.push_back({x, y+h});
    }
    
    IntervalSelector::IntervalSelector(float x_left, float x_right, float center_y, 
        float line_thickness, float handle_width, float handle_height) {
        vertices.reserve(6 * 4); // 4 rectangles

        float half_line = line_thickness * 0.5f;
        float half_handle_w = handle_width * 0.5f;
        float half_handle_h = handle_height * 0.5f;

        // Base Line (full width from -1 to +1)
        add_rect(vertices, x_left, center_y - half_line, x_right - x_left, line_thickness);

        // Selected Region
        add_rect(vertices, x_left, center_y - half_line, (x_right - x_left), line_thickness);

        // Left Handle
        add_rect(vertices, x_left - half_handle_w, center_y - half_handle_h, handle_width, handle_height);

        // Right Handles
        add_rect(vertices, x_right - half_handle_w, center_y - half_handle_h, handle_width, handle_height);
    }

    PolySelector::PolySelector(float x_top, float y_top, float width, float height) {
        add_rect(tri_vertices, x_top, y_top, width, height);
        
        pt_vertices.push_back(Vector2f(x_top, y_top));
        pt_vertices.push_back(Vector2f(x_top + width, y_top));
        pt_vertices.push_back(Vector2f(x_top, y_top + height));
        pt_vertices.push_back(Vector2f(x_top + width, y_top + height));
    }
}

