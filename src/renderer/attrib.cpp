#include <exception>
#include <format>
#include <algorithm>
#include <ranges>
#include "renderer.h"
#include "pipeline.h"

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
        glGenVertexArrays(1, &vao_interval);
        glGenBuffers(1, &vbo_x_axis);
        glGenBuffers(1, &vbo_y_axis);
        glGenBuffers(1, &vbo_marker);
        glGenBuffers(1, &vbo_marker_pos);
        glGenBuffers(1, &vbo_interval);
        
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
        
        glBindVertexArray(vao_interval);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_interval);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (void*)0);
        
        glBindVertexArray(0);
        
        setup_traits();

#ifdef MVF_DEBUG
        std::cout << "Created attribute space static mesh buffers..." << std::endl;
#endif
    }
    
    void AttribRenderer::setup_traits() {
        if (!traits.size()) {
            return;
        }
    
        // First, draw any marker points
        auto vertices_point = traits | std::views::filter([] (auto& trait) {
            return trait.type == TraitType::POINT;
        }) | std::views::transform([] (auto& trait) {return std::get<Point>(trait.data);})
        | std::ranges::to<std::vector>();
            
        glBindBuffer(GL_ARRAY_BUFFER, vbo_marker_pos);
        glBufferData(GL_ARRAY_BUFFER, vertices_point.size() * sizeof(Point), vertices_point.data(), GL_DYNAMIC_DRAW);
    
        // Next, build the interval selectors
        auto vertices_interval = traits | std::views::filter([] (auto& trait) {
            return trait.type == TraitType::RANGE && std::get<Range>(trait.data).type == RangeType::INTERVAL;
        }) | std::views::transform([] (auto& trait) {return std::get<Interval>(std::get<Range>(trait.data).range).mesh.vertices;})
        | std::views::join | std::ranges::to<std::vector>();
           
        num_interval_vertices = vertices_interval.size();
        glBindBuffer(GL_ARRAY_BUFFER, vbo_interval);
        glBufferData(GL_ARRAY_BUFFER, vertices_interval.size() * sizeof(Vector2f), vertices_interval.data(), GL_DYNAMIC_DRAW);
    }
    
    void AttribRenderer::set_field_data(std::shared_ptr<VolumeData>& vol) {
        data = vol;
        descriptors.clear();
        traits.clear();
    }
    
    void AttribRenderer::set_attrib_space_axis(const std::vector<AxisDesc>& descriptors) {
        if (!data || descriptors.size() > 2) {
            return;
        }

        this->descriptors.clear();
        traits.clear();
        for (auto& val: descriptors) {
            auto& comp = std::get<0>(data->scalars[val.comp_name]); 
            auto [min_val, max_val] = std::minmax_element(comp.begin(), comp.end());
#ifdef MVF_DEBUG
            std::cout << std::format("Field-{}: min_val={:.2f}, max_val={:.2f}", val.comp_name, *min_val, *max_val) << std::endl;
#endif
            this->descriptors.push_back(AxisDescMeta {.desc = val, .min_val = *min_val, .max_val = *max_val});
        }
    }
        
    std::pair<std::vector<AxisDesc>, std::vector<Trait>> AttribRenderer::get_traits() {
        auto desc = this->descriptors | std::views::transform([] (const AxisDescMeta& val) {
            return val.desc;
        }) | std::ranges::to<std::vector<AxisDesc>>();

        return std::make_pair(desc, traits);
    }
        
    void AttribRenderer::clear_traits() {
        traits.clear();
        num_interval_vertices = 0;
    }

    void AttribRenderer::render() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (!descriptors.size()) {
            return;
        }

        const Vector4f box_color = Vector4f(0, 0, 0, 1.0f);
		auto pipeline_axis = reinterpret_cast<AxisPipeline*>(pipelines[static_cast<int>(PipelineType::AXIS)]);
		glUseProgram(pipeline_axis->shader_program);
		glUniform4fv(pipeline_axis->uColor, 1, (float*)&box_color);

        glBindVertexArray(vao_x_axis);
        glDrawArrays(GL_TRIANGLES, 0, axis_mesh_x.vertices.size());

        if (descriptors.size() == 2) {
            glBindVertexArray(vao_y_axis);
            glDrawArrays(GL_TRIANGLES, 0, axis_mesh_y.vertices.size());
        }

        // Draw the traits
        auto point_traits = std::ranges::distance(traits | std::views::filter([] (auto& trait) {
            return trait.type == TraitType::POINT;
        }));

        if (point_traits) {
            // This should be drawn on top layer
            glClear(GL_DEPTH_BUFFER_BIT);
            const Vector4f marker_color = Vector4f(1.0f, 0, 0, 1.0f);
            auto pipeline_marker = reinterpret_cast<MarkerPipeline*>(pipelines[static_cast<int>(PipelineType::MARKER)]);
            glUseProgram(pipeline_marker->shader_program);
            glUniform4fv(pipeline_marker->uColor, 1, (float*)&marker_color);
            glBindVertexArray(vao_marker);
            glDrawArraysInstanced(GL_TRIANGLES, 0, marker.vertices.size(), point_traits);
        }
       
        if (num_interval_vertices) {
            const Vector4f interval_color = Vector4f(1.0f, 0.8f, 0, 0.5f);
            glUseProgram(pipeline_axis->shader_program);
            glUniform4fv(pipeline_axis->uColor, 1, (float*)&interval_color);

            glBindVertexArray(vao_interval);
            glDrawArrays(GL_TRIANGLES, 0, num_interval_vertices);
        }

        glBindVertexArray(0);
    }

    void AttribRenderer::resync() {
        setup_buffers();
    }
   
    void AttribRenderer::set_range_trait(float x1, float x2) {
        if (descriptors.size() != 1 || std::abs(x1) >= AXIS_LENGTH / 2 || std::abs(x2) >= AXIS_LENGTH / 2) {
            return;
        }

        traits.push_back(Trait {.type = TraitType::RANGE, .data = Range{.type = RangeType::INTERVAL, .range = Interval{.left = x1, .right = x2, 
            .mesh = IntervalSelector(x1, x2, 0, 0.01f, 0.03f, 0.1f)}}});
        setup_traits();
    }

    void AttribRenderer::set_point_trait(float x) {
        set_point_trait(x, 0);        
    }

    void AttribRenderer::set_point_trait(float x, float y) {
        if (descriptors.size() < 1 || std::abs(x) >= AXIS_LENGTH / 2 || std::abs(y) >= AXIS_LENGTH / 2) {
            return;
        }
        
        traits.push_back(Trait {.type = TraitType::POINT, .data = Point{.x = x, .y = y}});

        setup_traits();
    }
    
    float AttribRenderer::get_field_point(float t, size_t id) {
        if (descriptors.size() <= id) {
            throw std::runtime_error(std::format("get_field_point() called with id {} when descriptors.size() = {}", id, descriptors.size()));
        }

        auto& desc = descriptors[id];
        auto u_f = 0.5 * ((desc.max_val + desc.min_val) + t * (desc.max_val - desc.min_val));
    
        return u_f; 
    }
}
