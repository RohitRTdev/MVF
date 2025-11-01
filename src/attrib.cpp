#include "renderer.h"

namespace MVF {
    void AttribRenderer::init(int width, int height) {
		Renderer::init(width, height);
        pipelines = Pipeline::init_pipelines(false);
        
        glClearColor(0.984f, 0.894f, 0.913f, 1.0f);
        axis_mesh_x = Axis(10, 1.6f, 0.015f, 0.05f, true);
        axis_mesh_y = Axis(10, 1.6f, 0.015f, 0.05f, false);
    }

    void AttribRenderer::setup_buffers() {
        glGenVertexArrays(1,&vao_x_axis);
        glGenVertexArrays(1,&vao_y_axis);
        glGenBuffers(1, &vbo_x_axis);
        glGenBuffers(1, &vbo_y_axis);
        
        glBindVertexArray(vao_x_axis);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_x_axis);
        glBufferData(GL_ARRAY_BUFFER, axis_mesh_x.vertices.size() * sizeof(Vector2f), axis_mesh_x.vertices.data(), GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (void*)0);
        
        glBindVertexArray(vao_y_axis);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_y_axis);
        glBufferData(GL_ARRAY_BUFFER, axis_mesh_y.vertices.size() * sizeof(Vector2f), axis_mesh_y.vertices.data(), GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (void*)0);
        
        glBindVertexArray(0); 
#ifdef MVF_DEBUG
        std::cout << "Created axis mesh buffers..." << std::endl;
#endif
    }
    
    void AttribRenderer::set_field_data(std::shared_ptr<VolumeData>& vol) {
        data = vol;
    }
    
    void AttribRenderer::set_attrib_space_dim(const std::vector<AxisDesc>& descriptors) {
        if (descriptors.size() > 2) {
            return;
        }

        this->descriptors = descriptors;
    }

    void AttribRenderer::render() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (!descriptors.size()) {
            return;
        }

		auto pipeline_axis = reinterpret_cast<AxisPipeline*>(pipelines[static_cast<int>(PipelineType::AXIS)]);
		glUseProgram(pipeline_axis->shader_program);

        glBindVertexArray(vao_x_axis);
        glDrawArrays(GL_TRIANGLES, 0, axis_mesh_x.vertices.size());

        if (descriptors.size() == 2) {
            glBindVertexArray(vao_y_axis);
            glDrawArrays(GL_TRIANGLES, 0, axis_mesh_y.vertices.size());
        }
        glBindVertexArray(0);
    }

    void AttribRenderer::unload() {
        glDeleteVertexArrays(2, std::array{vao_x_axis, vao_y_axis}.data());
        glDeleteBuffers(2, std::array{vbo_x_axis, vbo_y_axis}.data());    
#ifdef MVF_DEBUG
        std::cout << "Deleted axis mesh buffers..." << std::endl;
#endif
    }
        
    void AttribRenderer::resync() {
        setup_buffers();
    }
}
