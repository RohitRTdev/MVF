#pragma once

#include <variant>
#include <atomic>
#include <mutex>
#include "vtk.h"
#include "math_utils.h"
#include "shapes.h"
#include "pipeline.h"
#include "attrib.h"
#include "widgets.h"

enum class EntityMode {
    NONE,
    VECTOR_GLYPH,
    SCALAR_SLICE,
    DVR
};

struct VectorGlyphDesc {
    std::string field1;
    std::string field2;
    std::string field3;
};

struct ScalarSliceDesc {
    std::string field;
    int axis; // 0=X,1=Y,2=Z
};

struct DVRDesc {
    std::string field; // scalar field for DVR (magnitude placeholder)
};

using EntityData = std::variant<VectorGlyphDesc, ScalarSliceDesc, DVRDesc>;

struct EntityRepresentation {
    EntityMode mode;
    EntityData data;
};

namespace MVF {
    class SpatialRenderer;
    class FieldRenderer;
    class FieldEntity;

    struct ProgressProxy {
        std::atomic<size_t> points_written;
        size_t total_points;
    };


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

        struct SliceBufferEntity {
            bool is_active = false;
            GLuint vao_slice = 0;
            GLuint vbo_slice = 0; // pos(3) + uv(2)
            GLuint tex_slice = 0; // GL_R8
            int tex_w = 0;
            int tex_h = 0;
        };

        struct DVRBufferEntity {
            bool is_active = false;
            GLuint vao = 0;
            GLuint vbo = 0; // quad XY, z computed in shader via instancing
            GLuint tex3d = 0; // 3D volume texture
            int nx = 0, ny = 0, nz = 0;
            int num_slices = 128;
        };

    public:
        VolumeEntity();
        void load_model(std::shared_ptr<VolumeData>& data);
        void destroy_buffers(bool destroy_box = true);
        void set_box_mode();
        void set_vector_mode(const std::string& field1, const std::string& field2);
        void set_vector_mode(const std::string& field1, const std::string& field2, const std::string& field3);
        void set_scalar_slice(const std::string& field, int axis = 2);
        void set_slice_position(float t);
        void set_slice_axis(int axis); // change axis without altering field
        int get_slice_axis() const;    // query current slice axis or -1 if not in slice mode
        float get_slice_position() const; // current normalized slice position
        void set_dvr(const std::string& field); // new: enable DVR on a scalar field
        EntityMode get_mode() const;
        void scale(float factor);
        void reset_transform(); 
        void resync();
        friend SpatialRenderer;
        friend FieldRenderer;
        friend FieldEntity;
    
    private:
        std::shared_ptr<VolumeData> model;
        Matrix4f init_transform;
        Matrix4f scale_transform;
        
        EntityRepresentation type = {.mode = EntityMode::NONE};    
        BoundingBox box; 
        ArrowMesh arrow_mesh;
        GlyphMesh field_mesh;
        ArrowBufferEntity arrow_buffer;
        BoxBufferEntity box_buffer;
        VectorBufferEntity vec_buffer;
        SliceBufferEntity slice_buffer;
        DVRBufferEntity dvr_buffer;

        bool initialized = false;
        float quad[12] = {
            -1.f, -1.f, 0.f, 1.f,
            -1.f, 0.f, -1.f, 1.f,
            0.f,  1.f,  1.f, 0.f
        };

        std::vector<std::string> fields;
        std::vector<Pipeline*> pipelines;
        int selectedField = -1;

        float slice_t = 0.5f; // [0,1]

        void compute_bounding_box();
        void init(std::vector<Pipeline*>& pipelines);
        void init_model_space();
        void create_vertex_array();
        void create_bounding_box_buffers(); 
        void create_buffers();
        void update_slice_resources();
        void update_dvr_resources();
        void draw() override;
    };

    class FieldEntity : Entity {
    public:
        FieldEntity();
        void init(VolumeEntity* geometry_entity);
        void set_traits(const std::vector<AxisDescMeta>& attrib_comps, const std::vector<Trait>& traits);
        void complete_set_traits();
        void cancel_dist_computation();
        void clear_traits();
        void set_isovalue(float value);
        void set_apply_color(bool apply_color);
        friend FieldRenderer;
    
    private:
    
        GLuint vao, vbo;
        GLuint tex3d, tex3d_col;
        Vector3f steps;
        const size_t res_x = 100, res_y = 100, res_z = 100;
        float iso_value = 0;
        std::vector<Vertex> points;
        std::vector<float> field;
        std::vector<Vector3f> color_field;
        std::vector<AxisDescMeta> attrib_comps;
        std::vector<Trait> traits;
        VolumeEntity* geometry_entity;
        std::mutex dist_fld_lock;
        std::thread worker_thread; 
        bool set_draw_mode = false;
        bool compute_passed = true; 
        bool is_apply_color = false;
        std::atomic<bool> stop_requested = false; 

        void create_voxel_grid();
        void create_buffers();
        void build_distance_field();
        void build_texture();
        
        void draw() override;
    };

    class CameraEntity : public Entity {
    public:
        CameraEntity(const Vector3f& eye, const Vector3f& target, const Vector3f& up);
        void translate(float x, float y, float z) override;
        void rotate(float angleX, float angleY, float angleZ) override;

        friend SpatialRenderer;
        friend FieldRenderer;
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
