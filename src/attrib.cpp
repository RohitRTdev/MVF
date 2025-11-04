#include <exception>
#include <format>
#include "renderer.h"


namespace MVF {
    void AttribRenderer::init(int width, int height) {
		Renderer::init(width, height);
        pipelines = Pipeline::init_pipelines(false);
        
        glClearColor(0.984f, 0.894f, 0.913f, 1.0f);
        axis_mesh_x = Axis(10, 1.6f, 0.015f, 0.05f, true);
        axis_mesh_y = Axis(10, 1.6f, 0.015f, 0.05f, false);
        marker = PointMarker(0.08f, 0.01f);
    }

    void AttribRenderer::setup_buffers() {
        glGenVertexArrays(1, &vao_x_axis);
        glGenVertexArrays(1, &vao_y_axis);
        glGenVertexArrays(1, &vao_marker);
        glGenBuffers(1, &vbo_x_axis);
        glGenBuffers(1, &vbo_y_axis);
        glGenBuffers(1, &vbo_marker);
        glGenBuffers(1, &vbo_marker_pos);
        
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
        
        glBindVertexArray(vao_marker);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_marker);
        glBufferData(GL_ARRAY_BUFFER, marker.vertices.size() * sizeof(Vector2f), marker.vertices.data(), GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (void*)0);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo_marker_pos);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), 0);
        glVertexAttribDivisor(1, 1);
        
        glBindVertexArray(0); 
#ifdef MVF_DEBUG
        std::cout << "Created attribute space static mesh buffers..." << std::endl;
#endif
    }
    
    void AttribRenderer::set_field_data(std::shared_ptr<VolumeData>& vol) {
        data = vol;
        descriptors.clear();
    }
    
    void AttribRenderer::set_attrib_space_axis(const std::vector<AxisDesc>& descriptors) {
        if (!data || descriptors.size() > 2) {
            return;
        }

        this->descriptors.clear();
        for (auto& val: descriptors) {
            auto min_val = INFINITY, max_val = -INFINITY;
            for (auto& point: std::get<0>(data->scalars[val.comp_name])) {
                min_val = std::min(point, min_val);
                max_val = std::max(point, max_val);
            }

            this->descriptors.push_back(AxisDescMeta {.desc = val, .min_val = min_val, .max_val = max_val});
        }
    }

    void AttribRenderer::render() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (!descriptors.size()) {
            return;
        }

        const Vector3f box_color = Vector3f(0, 0, 0);
		auto pipeline_axis = reinterpret_cast<AxisPipeline*>(pipelines[static_cast<int>(PipelineType::AXIS)]);
		glUseProgram(pipeline_axis->shader_program);
		glUniform3fv(pipeline_axis->uColor, 1, (float*)&box_color);

        glBindVertexArray(vao_x_axis);
        glDrawArrays(GL_TRIANGLES, 0, axis_mesh_x.vertices.size());

        if (descriptors.size() == 2) {
            glBindVertexArray(vao_y_axis);
            glDrawArrays(GL_TRIANGLES, 0, axis_mesh_y.vertices.size());
        }

        // Draw point traits for now
        if (traits.size()) {
            // This should be drawn on top layer
            glClear(GL_DEPTH_BUFFER_BIT);
            const Vector3f marker_color = Vector3f(1.0f, 0, 0);
            auto pipeline_marker = reinterpret_cast<MarkerPipeline*>(pipelines[static_cast<int>(PipelineType::MARKER)]);
            glUseProgram(pipeline_marker->shader_program);
            glUniform3fv(pipeline_marker->uColor, 1, (float*)&marker_color);
            glBindVertexArray(vao_marker);
            glDrawArraysInstanced(GL_TRIANGLES, 0, marker.vertices.size(), traits.size());
        }

        glBindVertexArray(0);
    }

    void AttribRenderer::unload() {
        glDeleteVertexArrays(3, std::array{vao_x_axis, vao_y_axis, vao_marker}.data());
        glDeleteBuffers(3, std::array{vbo_x_axis, vbo_y_axis, vbo_marker}.data());    
#ifdef MVF_DEBUG
        std::cout << "Deleted axis mesh buffers..." << std::endl;
#endif
    }
        
    void AttribRenderer::resync() {
        setup_buffers();
    }
    
    void AttribRenderer::set_point_trait(float x) {
        set_point_trait(x, 0);        
    }

    void AttribRenderer::set_point_trait(float x, float y) {
        if (descriptors.size() < 1 || std::abs(x) >= AXIS_LENGTH / 2 || std::abs(y) >= AXIS_LENGTH / 2) {
            return;
        }
        
        traits.push_back(Trait {.type = TraitType::POINT, .data = Point{.x = x, .y = y}});

        std::vector<Point> vertices;
        for (auto& trait: traits) {
            vertices.push_back(std::get<Point>(trait.data));
        }
            
        glBindBuffer(GL_ARRAY_BUFFER, vbo_marker_pos);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Point), vertices.data(), GL_DYNAMIC_DRAW);
    }
    
    std::pair<float, float> AttribRenderer::get_field_point(float x, float y, size_t id) {
        if (descriptors.size() <= id) {
            throw std::runtime_error(std::format("get_field_point() called with id {} when descriptors.size() = {}", id, descriptors.size()));
        }

        auto& desc = descriptors[id];
        auto x_f = 0.5 * ((desc.max_val + desc.min_val) + x * (desc.max_val - desc.min_val));
        auto y_f = 0.5 * ((desc.max_val + desc.min_val) + y * (desc.max_val - desc.min_val));
    
        return std::make_pair(x_f, y_f);
    }
}
