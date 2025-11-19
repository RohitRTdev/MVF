#include <exception>
#include <format>
#include <algorithm>
#include <ranges>
#include "renderer.h"
#include "pipeline.h"

namespace MVF {
    void AttribRenderer::init(int width, int height) {
		Renderer::init(width, height);
		glDisable(GL_DEPTH_TEST); 
        pipelines = Pipeline::init_pipelines(false);
        
        glClearColor(0.984f, 0.894f, 0.913f, 1.0f);
        axis_mesh_x = Axis(10, AXIS_LENGTH, AXIS_WIDTH, 0.05f, true);
        axis_mesh_y = Axis(10, AXIS_LENGTH, AXIS_WIDTH, 0.05f, false);
        marker = PointMarker(0.08f, 0.01f);
    }

    void AttribRenderer::setup_buffers() {
        glGenVertexArrays(1, &vao_x_axis);
        glGenVertexArrays(1, &vao_y_axis);
        glGenVertexArrays(1, &vao_marker);
        glGenVertexArrays(1, &vao_interval);
        glGenVertexArrays(1, &vao_polyline);
        glGenVertexArrays(1, &vao_polypoint);
        glGenVertexArrays(1, &vao_scatterplot);
        glGenVertexArrays(1, &vao_distplotsolid);
        glGenVertexArrays(1, &vao_distplotlines);

        glGenBuffers(1, &vbo_x_axis);
        glGenBuffers(1, &vbo_y_axis);
        glGenBuffers(1, &vbo_marker);
        glGenBuffers(1, &vbo_marker_pos);
        glGenBuffers(1, &vbo_interval);
        glGenBuffers(1, &vbo_polyline);
        glGenBuffers(1, &vbo_polypoint);
        glGenBuffers(1, &vbo_scatterplot);
        glGenBuffers(1, &vbo_distplotsolid);
        glGenBuffers(1, &vbo_distplotlines);
        
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
        
        glBindVertexArray(vao_polyline);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_polyline);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (void*)0);
        
        glBindVertexArray(vao_polypoint);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_polypoint);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (void*)0);
        
        glBindVertexArray(vao_scatterplot);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_scatterplot);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (void*)0);

        glBindVertexArray(vao_distplotsolid);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_distplotsolid);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (void*)0);
        
        glBindVertexArray(vao_distplotlines);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_distplotlines);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (void*)0);

        glBindVertexArray(0);
        setup_traits();
        setup_plot();

#ifdef MVF_DEBUG
        std::cout << "Created attribute space static mesh buffers..." << std::endl;
#endif
    }
        
    void AttribRenderer::setup_plot() {
        if (scatter_plot.size()) {
            glBindBuffer(GL_ARRAY_BUFFER, vbo_scatterplot);
            glBufferData(GL_ARRAY_BUFFER, scatter_plot.size() * sizeof(Vector2f), scatter_plot.data(), GL_DYNAMIC_DRAW);
        }
        else if (dist_plot_solid.size()) {
            glBindBuffer(GL_ARRAY_BUFFER, vbo_distplotsolid);
            glBufferData(GL_ARRAY_BUFFER, dist_plot_solid.size() * sizeof(Vector2f), dist_plot_solid.data(), GL_DYNAMIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, vbo_distplotlines);
            glBufferData(GL_ARRAY_BUFFER, dist_plot_lines.size() * sizeof(Vector2f), dist_plot_lines.data(), GL_DYNAMIC_DRAW);
        }
    }
    
    void AttribRenderer::setup_traits() {
        if (!traits.size()) {
            return;
        }
   
        if (!descriptors.size()) {
            throw std::runtime_error("setup_traits() called when descriptors.size() == 0");
        }

        // First, draw any marker points
        auto vertices_point = traits | std::views::filter([] (auto& trait) {
            return trait.type == TraitType::POINT;
        }) | std::views::transform([] (auto& trait) {return std::get<Point>(trait.data);})
        | std::ranges::to<std::vector>();
            
        glBindBuffer(GL_ARRAY_BUFFER, vbo_marker_pos);
        glBufferData(GL_ARRAY_BUFFER, vertices_point.size() * sizeof(Point), vertices_point.data(), GL_DYNAMIC_DRAW);
    
        // Next, build the range selectors 
        switch(descriptors.size()) {
            case 1: {
                auto vertices_interval = traits | std::views::filter([] (auto& trait) {
                    return trait.type == TraitType::RANGE && std::get<Range>(trait.data).type == RangeType::INTERVAL;
                }) | std::views::transform([] (auto& trait) {return std::get<Interval>(std::get<Range>(trait.data).range).mesh.vertices;})
                | std::views::join | std::ranges::to<std::vector>();
                
                num_interval_vertices = vertices_interval.size();
                glBindBuffer(GL_ARRAY_BUFFER, vbo_interval);
                glBufferData(GL_ARRAY_BUFFER, vertices_interval.size() * sizeof(Vector2f), vertices_interval.data(), GL_DYNAMIC_DRAW);

                break;
            }
            case 2: {
                auto pt_vertices = traits | std::views::filter([] (auto& trait) {
                    return trait.type == TraitType::RANGE && std::get<Range>(trait.data).type == RangeType::POLYGON;
                }) | std::views::transform([] (auto& trait) {return std::get<Polygon>(std::get<Range>(trait.data).range).mesh.pt_vertices;})
                | std::views::join | std::ranges::to<std::vector>();

                auto tri_vertices = traits | std::views::filter([] (auto& trait) {
                    return trait.type == TraitType::RANGE && std::get<Range>(trait.data).type == RangeType::POLYGON;
                }) | std::views::transform([] (auto& trait) {return std::get<Polygon>(std::get<Range>(trait.data).range).mesh.tri_vertices;})
                | std::views::join | std::ranges::to<std::vector>();


                glBindBuffer(GL_ARRAY_BUFFER, vbo_polyline);
                glBufferData(GL_ARRAY_BUFFER, tri_vertices.size() * sizeof(Vector2f), tri_vertices.data(), GL_DYNAMIC_DRAW);

                glBindBuffer(GL_ARRAY_BUFFER, vbo_polypoint);
                glBufferData(GL_ARRAY_BUFFER, pt_vertices.size() * sizeof(Vector2f), pt_vertices.data(), GL_DYNAMIC_DRAW);

                num_range_tri_vertices = tri_vertices.size();
                num_range_pt_vertices = pt_vertices.size();
                break;
            }
        }
    }
    
    void AttribRenderer::set_field_data(std::shared_ptr<VolumeData>& vol) {
        data = vol;
        descriptors.clear();
        clear_traits();
        clear_plot();
    }
    
    void AttribRenderer::set_attrib_space_axis(const std::vector<AxisDesc>& descriptors) {
        if (!data || descriptors.size() > 2) {
            return;
        }

        this->descriptors.clear();
        clear_traits();
        clear_plot();
        for (auto& val: descriptors) {
            auto& comp = std::get<0>(data->scalars[val.comp_name]); 
            auto [min_val, max_val] = std::minmax_element(comp.begin(), comp.end());
#ifdef MVF_DEBUG
            std::cout << std::format("Field-{}: min_val={:.2f}, max_val={:.2f}", val.comp_name, *min_val, *max_val) << std::endl;
#endif
            this->descriptors.push_back(AxisDescMeta {.desc = val, .min_val = *min_val, .max_val = *max_val});
        }

        if (descriptors.size() == 1) {
            generate_freq_distribution();
        }
        else if (descriptors.size() == 2) {
            generate_scatter_plot();
        }
    }
           
    void AttribRenderer::enable_plot(bool enable) {
        if (enable) {
            setup_plot();
        }

        is_plot_visible = enable;
    }

    std::pair<std::vector<AxisDescMeta>, std::vector<Trait>> AttribRenderer::get_traits() {
        return std::make_pair(descriptors, traits);
    }
        
    void AttribRenderer::clear_traits() {
        traits.clear();
        num_interval_vertices = 0;
        num_range_pt_vertices = 0;
        num_range_tri_vertices = 0;
    }

    void AttribRenderer::render() {
		glClear(GL_COLOR_BUFFER_BIT);

        if (!descriptors.size()) {
            return;
        }

        const Vector4f box_color = Vector4f(0, 0, 0, 1.0f);
		auto pipeline_axis = reinterpret_cast<AxisPipeline*>(pipelines[static_cast<int>(PipelineType::AXIS)]);
		glUseProgram(pipeline_axis->shader_program);
		glUniform4fv(pipeline_axis->uColor, 1, (float*)&box_color);

        glBindVertexArray(vao_x_axis);
        glDrawArrays(GL_TRIANGLES, 0, axis_mesh_x.vertices.size());

        if (descriptors.size() == 1) {
            // Draw the distribution plot
            if (is_plot_visible && dist_plot_solid.size()) {
                const Vector4f rect_color = Vector4f(1.0, 0, 1.0, 1.0f);
                glUniform4fv(pipeline_axis->uColor, 1, (float*)&rect_color);

                // First, draw the rectangle
                glBindVertexArray(vao_distplotsolid);
                glDrawArrays(GL_TRIANGLES, 0, dist_plot_solid.size());
                
                // Now, draw the outline
                const Vector4f line_color = Vector4f(0, 0, 0, 1.0f);
                glUniform4fv(pipeline_axis->uColor, 1, (float*)&line_color);
                glBindVertexArray(vao_distplotlines);
                glDrawArrays(GL_LINES, 0, dist_plot_lines.size());
            }
        }
        else if (descriptors.size() == 2) {
            glBindVertexArray(vao_y_axis);
            glDrawArrays(GL_TRIANGLES, 0, axis_mesh_y.vertices.size());
        
            // Draw the scatter plot
            if (is_plot_visible && scatter_plot.size()) {
                const Vector4f point_color = Vector4f(1.0, 0, 1.0, 1.0f);
                glUniform4fv(pipeline_axis->uColor, 1, (float*)&point_color);

                glBindVertexArray(vao_scatterplot);
                glDrawArrays(GL_POINTS, 0, scatter_plot.size());
            }
        }

        // Draw the traits
        auto point_traits = std::ranges::distance(traits | std::views::filter([] (auto& trait) {
            return trait.type == TraitType::POINT;
        }));

        // Point traits will be drawn on bottom layer
        if (point_traits) {
            const Vector4f marker_color = Vector4f(1.0f, 0, 0, 1.0f);
            auto pipeline_marker = reinterpret_cast<MarkerPipeline*>(pipelines[static_cast<int>(PipelineType::MARKER)]);
            glUseProgram(pipeline_marker->shader_program);
            glUniform4fv(pipeline_marker->uColor, 1, (float*)&marker_color);
            glBindVertexArray(vao_marker);
            glDrawArraysInstanced(GL_TRIANGLES, 0, marker.vertices.size(), point_traits);
        }
       
        if (num_interval_vertices) {
            const Vector4f interval_color = Vector4f(1.0f, 0.8f, 0, 0.8f);
            glUseProgram(pipeline_axis->shader_program);
            glUniform4fv(pipeline_axis->uColor, 1, (float*)&interval_color);

            glBindVertexArray(vao_interval);
            glDrawArrays(GL_TRIANGLES, 0, num_interval_vertices);
        } else if (num_range_tri_vertices) {
            Vector4f line_color = Vector4f(1.0f, 0.8f, 0, 0.8f);
            glUseProgram(pipeline_axis->shader_program);
            glUniform4fv(pipeline_axis->uColor, 1, (float*)&line_color);

            // First draw the range rectangle
            glBindVertexArray(vao_polyline);
            glDrawArrays(GL_TRIANGLES, 0, num_range_tri_vertices);

            line_color = Vector4f(1.0f, 0.2f, 0, 1.0f);
            glUseProgram(pipeline_axis->shader_program);
            glUniform4fv(pipeline_axis->uColor, 1, (float*)&line_color);
        
            // Now, draw the corner points
            glBindVertexArray(vao_polypoint);
            glDrawArrays(GL_POINTS, 0, num_range_pt_vertices);
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
    
    void AttribRenderer::modify_range_trait(float x_right) {
        if (traits.size() == 0) {
            throw std::runtime_error("Called modify_range_trait() with traits.size() == 0");
        }

        auto& last_trait = traits[traits.size() - 1];
        if (last_trait.type != TraitType::RANGE || std::get<Range>(last_trait.data).type != RangeType::INTERVAL) {
            throw std::runtime_error("Called modify_range_trait() on interval trait, but last added trait was not interval...");
        }
        
        if (descriptors.size() != 1 || std::abs(x_right) >= AXIS_LENGTH / 2) {
            return;
        }

        auto saved_x_left = std::get<Interval>(std::get<Range>(last_trait.data).range).left;  
        std::get<Interval>(std::get<Range>(last_trait.data).range).right = x_right;
        std::get<Interval>(std::get<Range>(last_trait.data).range).mesh = IntervalSelector(saved_x_left, x_right, 0, 0.01f, 0.03f, 0.1f);
        
        setup_traits();
    }
    
    void AttribRenderer::set_range_trait(float x_top, float y_top, float width, float height) {
        if (descriptors.size() != 2 || std::abs(x_top) >= AXIS_LENGTH / 2 || std::abs(y_top) >= AXIS_LENGTH / 2 
            || std::abs(x_top + width) >= AXIS_LENGTH / 2 || std::abs(y_top + height) >= AXIS_LENGTH / 2) {
            return;
        }

        traits.push_back(Trait {.type = TraitType::RANGE, .data = Range{.type = RangeType::POLYGON, .range = Polygon{.x_top = x_top,
            .y_top = y_top, .width = width, .height = height, .mesh = PolySelector(x_top, y_top, width, height)}}});

        setup_traits();
    }

    void AttribRenderer::modify_range_trait(float x_end, float y_end) {
        if (traits.size() == 0) {
            throw std::runtime_error("Called modify_range_trait() with traits.size() == 0");
        }

        auto& last_trait = traits[traits.size() - 1];
        if (last_trait.type != TraitType::RANGE || std::get<Range>(last_trait.data).type != RangeType::POLYGON) {
            throw std::runtime_error("Called modify_range_trait() on polygon trait, but last added trait was not polygon...");
        }
        
        if (descriptors.size() != 2 || std::abs(x_end) >= AXIS_LENGTH / 2 || std::abs(y_end) >= AXIS_LENGTH / 2) {
            return;
        }

        auto saved_x_top = std::get<Polygon>(std::get<Range>(last_trait.data).range).x_top;
        auto saved_y_top = std::get<Polygon>(std::get<Range>(last_trait.data).range).y_top;

        auto width = x_end - saved_x_top;
        auto height = y_end - saved_y_top; 

        std::get<Polygon>(std::get<Range>(last_trait.data).range).width = width;
        std::get<Polygon>(std::get<Range>(last_trait.data).range).height = height;
        std::get<Polygon>(std::get<Range>(last_trait.data).range).mesh = PolySelector(saved_x_top, saved_y_top, width, height);

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
        auto u_norm = t / (AXIS_LENGTH / 2);
        auto u_f = 0.5 * ((desc.max_val + desc.min_val) + u_norm * (desc.max_val - desc.min_val));
    
        return u_f;
    }
}
