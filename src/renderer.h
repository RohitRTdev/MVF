#pragma once

#include <GL/glew.h>
#include <vector>

#include "entity.h"
#include "math_utils.h"
#include "attrib.h"

namespace MVF {
    class Renderer {
    public:
        Renderer();
        virtual void render() = 0;
        virtual void resync() = 0;
        virtual void unload() = 0;
        virtual void init(int width, int height);
        void set_viewport(int width, int height);
    
    protected:
        CameraEntity camera;
        LightEntity light;
        Matrix4f projection;
        std::vector<Pipeline*> pipelines;

    private:
        int width, height;
    };

    class SpatialRenderer : public Renderer {
    public:
        VolumeEntity entity;
        
        void init(int width, int height) override;
        void setup_scene(std::shared_ptr<VolumeData>& data);
        void purge_scene(); 

        void render() override;
        void resync() override;
        void unload() override;
    
    private:
        bool is_scene_setup = false;
    };

    class AttribRenderer : public Renderer {
    public:
        
        void init(int width, int height) override;

        void unload() override;
        void render() override;
        void resync() override;
    
        void set_field_data(std::shared_ptr<VolumeData>& vol);
        void set_attrib_space_dim(const std::vector<AxisDesc>& descriptors);

    private:
        Axis axis_mesh_x, axis_mesh_y;
        GLuint vao_x_axis, vao_y_axis, vbo_x_axis, vbo_y_axis;
        std::shared_ptr<VolumeData> data;
        std::vector<AxisDesc> descriptors;
        std::optional<Trait> sel_trait;
        void setup_buffers(); 
    };
}
