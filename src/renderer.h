#pragma once

#include <GL/glew.h>
#include <vector>

#include "entity.h"
#include "math_utils.h"

namespace MVF {
    class Renderer {
    public:
        VolumeEntity entity;
        CameraEntity camera;
        LightEntity light;

        Renderer();
        void init(int width, int height);
        void set_viewport(int width, int height);
        void setup_scene(std::shared_ptr<VolumeData>& data);
        void purge_scene(); 
        void render();
        
    private:
        int width, height;
        bool is_scene_setup = false;
        Matrix4f projection;
    };
}
