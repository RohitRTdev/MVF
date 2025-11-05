#pragma once

#include <variant>
#include "vtk.h"
#include "math_utils.h"
#include "shapes.h"
#include "pipeline.h"

enum class EntityMode {
    NONE,
    VECTOR_GLYPH
};

struct VectorGlyphDesc {
    std::string field1;
    std::string field2;
    std::string field3;
};

using EntityData = std::variant<VectorGlyphDesc>;

struct EntityRepresentation {
    EntityMode mode;
    EntityData data;
};

namespace MVF {
    class SpatialRenderer;

    class Entity {
    public:
        
        virtual void rotate(float angleX, float angleY, float angleZ);
        virtual void translate(float x, float y, float z);
        virtual Vector3f get_position();

        Entity(const Vector3f& position);
        friend SpatialRenderer;
    protected:
        Matrix4f world;
        Vector3f position; 
        
        float* get_world_transform();
        
        virtual void draw() {};
    };

    class VolumeEntity : public Entity { 
        struct ArrowBufferEntity {
            bool is_active = false;
            GLuint vao_vec_glyph, vbo_arrow_mesh, ebo_arrow_mesh;
        };
        
        struct BoxBufferEntity {
            bool is_active = false;
            GLuint vao_bound_box, vbo_box, ebo_box;
        };

        struct VectorBufferEntity {
            bool is_active = false;
            GLuint vbo_glyph;
        };

    public:
        VolumeEntity();
        void load_model(std::shared_ptr<VolumeData>& data);
        void destroy_buffers(bool destroy_static_buffers = true);
        void switch_field();
        void set_box_mode();
        void set_vector_mode(const std::string& field1, const std::string& field2, const std::string& field3);
        void scale(float factor);
        void resync();
        friend SpatialRenderer;
    
    private:
        std::shared_ptr<VolumeData> model;
        EntityRepresentation type;    
        BoundingBox box; 
        ArrowMesh arrow_mesh;
        GlyphMesh field_mesh;
        Matrix4f init_transform;
        Matrix4f scale_transform;
        ArrowBufferEntity arrow_buffer;
        BoxBufferEntity box_buffer;
        VectorBufferEntity vec_buffer;

        bool initialized = false;

        std::vector<std::string> fields;
        std::vector<Pipeline*> pipelines;
        int selectedField = -1;

        void compute_bounding_box();
        void init(std::vector<Pipeline*>& pipelines);
        void init_model_space();
        void create_vertex_array();
        void create_bounding_box_buffers(); 
        void create_buffers();

        void draw() override;
    };

    class CameraEntity : public Entity {
    public:
        CameraEntity(const Vector3f& eye, const Vector3f& target, const Vector3f& up);
        void translate(float x, float y, float z) override;
        void rotate(float angleX, float angleY, float angleZ) override;

        friend SpatialRenderer;
    private:
        Vector3f target;
        Vector3f up;
        Matrix4f view;
        
        void compute_view_matrix();
    };

    class LightEntity : public Entity {
    public:
        LightEntity(const Vector3f& position, const Vector3f& color);
    private:
        Vector3f color;
    };

}
