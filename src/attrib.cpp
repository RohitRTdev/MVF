#include "renderer.h"

namespace MVF {
    void AttribRenderer::init(int width, int height) {
		Renderer::init(width, height);
        pipelines = Pipeline::init_pipelines(false);
        
        glClearColor(0.984f, 0.894f, 0.913f, 1.0f);
        axis_mesh = Axis(10, 1.6f, 0.015f, 0.05f);
        setup_buffers();
    }

    void AttribRenderer::setup_buffers() {
        glGenVertexArrays(1,&VAO);
        glGenBuffers(1,&VBO);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, axis_mesh.vertices.size() * sizeof(Vector2f), axis_mesh.vertices.data(), GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (void*)0);
        
        glBindVertexArray(0); 
    }

    void AttribRenderer::render() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto pipeline_axis = reinterpret_cast<AxisPipeline*>(pipelines[static_cast<int>(PipelineType::AXIS)]);
		glUseProgram(pipeline_axis->shader_program);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, axis_mesh.vertices.size());
        glBindVertexArray(0);
    }

    void AttribRenderer::unload() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);    
    }
        
    void AttribRenderer::resync() {
        setup_buffers();
    }
}
