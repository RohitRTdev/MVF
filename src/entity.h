#pragma once

#include "vtk.h"
#include "math_utils.h"
#include "shapes.h"

namespace MVF {
    class Renderer;

    class Entity {
    public:
        virtual void rotate(float angleX, float angleY, float angleZ);
        virtual void translate(float x, float y, float z);
        virtual Vector3f get_position();

        Entity(const Vector3f& position);
        friend Renderer;
    protected:
        Matrix4f world;
        Vector3f position; 
        
        float* get_world_transform();
        
        virtual void draw() {};
    };

    class VolumeEntity : public Entity { 
    public:
        VolumeEntity();
        void load_model(std::shared_ptr<VolumeData>& data);
        void switch_field();
        void scale(float factor);
        void resync();
        friend Renderer;
    
    private:
        std::shared_ptr<VolumeData> model;    
        BoundingBox box; 
        ArrowMesh arrow_mesh;
        GlyphMesh field_mesh;
        Matrix4f init_transform;
        Matrix4f scale_transform;
        GLuint vao_vec_glyph, vao_bound_box;
        GLuint vbo_arrow_mesh, vbo_box, vbo_glyph;
        GLuint ebo_arrow_mesh, ebo_box;

        bool initialized = false;

        std::vector<std::string> fields;
        int selectedField = -1;

        void compute_bounding_box();
        void init();
        void init_model_space();
        void create_vertex_array();
        void create_buffers();

        void draw() override;
    };

    class CameraEntity : public Entity {
    public:
        CameraEntity(const Vector3f& eye, const Vector3f& target, const Vector3f& up);
        void translate(float x, float y, float z) override;
        void rotate(float angleX, float angleY, float angleZ) override;

        friend Renderer;
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
