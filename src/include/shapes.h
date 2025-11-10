#pragma once
#include <vector>
#include <cstdint>
#include <array>
#include <string>
#include "math_utils.h"
#include "vtk.h"

namespace MVF {
#pragma pack(push, 1)
    struct Vertex {
        float x, y, z;

        operator Vector3f() const {
            return Vector3f(x, y, z);
        }
    };
    
    struct ArrowVertex {
        float x, y, z; // Position
        float u, v, w; // Normal
    };

    struct GlyphInstance {
        Vector3f position;
        Vector3f direction;
        float scale;
    };
#pragma pack(pop)

    struct ArrowMesh {
        std::vector<ArrowVertex> vertices;
        std::vector<uint32_t> indices;

        ArrowMesh() = default;
        ArrowMesh(float rad_cyl, float rad_cone, float len_cyl, float len_cone);    
    private:
        float radius_cylinder;
        float radius_cone;
        float length_cylinder;
        float length_cone;

        void compute_normals();
    };

    struct BoundingBox {
        std::array<Vertex, 8> vertices;
        std::array<uint32_t, 12 * 2> indices; // 12 lines each requiring 2 vertices

        BoundingBox() = default;
        BoundingBox(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax);
    };

    struct GlyphMesh {
        std::vector<GlyphInstance> points;

        GlyphMesh() = default;
        GlyphMesh(VolumeData* model, const std::string& field1, const std::string& field2, const std::string& field3);
    };

    struct Axis {
        std::vector<Vector2f> vertices;

        Axis() = default;
        Axis(int tick_count, float axis_len, float axis_thickness, float tick_size, bool is_horizontal);
    
    private:
        void push_line_quad(Vector2f& a, Vector2f& b, float thickness);
        void push_arrow(Vector2f& base, Vector2f& dir, float length, float width);
    };

    struct PointMarker {
        std::vector<Vector2f> vertices;

        PointMarker() = default;
        PointMarker(float length, float thickness);
    
    private:
        Vector2f rotate_line(Vector2f& pt, float angle);
    };
}