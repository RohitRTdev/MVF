#include <exception>
#include <format>
#include <algorithm>
#include <ranges>
#include <limits>
#include "renderer.h"
#include "pipeline.h"
#include <gtk/gtk.h>
#include <pango/pangocairo.h>

namespace MVF {
    const Vector3f global_color_pallete[MAX_COLORS] = {
        Vector3f(1.0, 0.0, 0.0),
        Vector3f(0.0, 0.0, 1.0),
        Vector3f(0.0, 1.0, 0.0),
        Vector3f(1.0, 1.0, 0.0)
    };

    size_t AttribRenderer::get_color_id() {
        auto id = last_chosen_id;
        last_chosen_id = (last_chosen_id + 1) % MAX_COLORS;
        
        return id;
    }

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
        glGenVertexArrays(1, &vao_parallel_axes);
        glGenVertexArrays(1, &vao_parallel_lines);
        glGenVertexArrays(1, &vao_parallel_lines_highlight);
        glGenVertexArrays(1, &vao_parallel_lines_region);
        glGenVertexArrays(1, &vao_parallel_region_fill);

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
        glGenBuffers(1, &vbo_parallel_axes);
        glGenBuffers(1, &vbo_parallel_lines);
        glGenBuffers(1, &vbo_parallel_lines_highlight);
        glGenBuffers(1, &vbo_parallel_lines_region);
        glGenBuffers(1, &vbo_parallel_region_fill);
        
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
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Point), 0);
        glVertexAttribDivisor(1, 1);
        
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)sizeof(Vector2f));
        glVertexAttribDivisor(2, 1);
        
        glBindVertexArray(vao_interval);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_interval);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
        
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)sizeof(Vector2f));
        
        glBindVertexArray(vao_polyline);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_polyline);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
        
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)sizeof(Vector2f));
        
        glBindVertexArray(vao_polypoint);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_polypoint);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
        
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)sizeof(Vector2f));
        
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

        glBindVertexArray(vao_parallel_axes);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_axes);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (void*)0);

        glBindVertexArray(vao_parallel_lines);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_lines);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (void*)0);

        glBindVertexArray(vao_parallel_lines_highlight);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_lines_highlight);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)sizeof(Vector2f));

        glBindVertexArray(vao_parallel_lines_region);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_lines_region);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)sizeof(Vector2f));

        glBindVertexArray(vao_parallel_region_fill);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_region_fill);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)sizeof(Vector2f));

        glBindVertexArray(0);
        setup_parallel_coordinates();
        setup_pending_markers();
        setup_traits();
        setup_plot();
#ifdef MVF_DEBUG
        std::cout << "Created attribute space static mesh buffers..." << std::endl;
#endif
    }
   
    void AttribRenderer::setup_pending_markers() {
        if (parallel_pending_marker_positions.empty()) {
            return;
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo_marker_pos);
        glBufferData(GL_ARRAY_BUFFER, parallel_pending_marker_positions.size() * sizeof(Point), parallel_pending_marker_positions.data(), GL_DYNAMIC_DRAW);
    }
    
    void AttribRenderer::setup_parallel_coordinates() {
        if (parallel_axes_vertices.empty()) {
            return;
        }
        glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_axes);
        glBufferData(GL_ARRAY_BUFFER, parallel_axes_vertices.size() * sizeof(Vector2f), parallel_axes_vertices.data(), GL_STATIC_DRAW);
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
        else if (parallel_lines_vertices.size()) {
            glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_lines);
            glBufferData(GL_ARRAY_BUFFER, parallel_lines_vertices.size() * sizeof(Vector2f), parallel_lines_vertices.data(), GL_DYNAMIC_DRAW);
        }
    }
    
    void AttribRenderer::setup_traits() {
        if (!traits.size()) {
            return;
        }
        if (!descriptors.size()) {
            throw std::runtime_error("setup_traits() called when descriptors.size() == 0");
        }
        
        // Draw point traits for 1 and 2d case
        if (descriptors.size() <= 2) {
            auto vertices_point = traits | std::views::filter([] (auto& trait) {
                return trait.type == TraitType::POINT;
            }) | std::views::transform([] (auto& trait) {return std::get<Point>(trait.data);} )
            | std::ranges::to<std::vector>();
            glBindBuffer(GL_ARRAY_BUFFER, vbo_marker_pos);
            glBufferData(GL_ARRAY_BUFFER, vertices_point.size() * sizeof(Point), vertices_point.data(), GL_DYNAMIC_DRAW);
        }

        switch(descriptors.size()) {
            case 1: {
                // Draw range trait for 1d case
                auto vertices_interval = traits | std::views::filter([] (auto& trait) {
                    return trait.type == TraitType::RANGE && std::get<Range>(trait.data).type == RangeType::INTERVAL;
                }) | std::views::transform([] (auto& trait) {return std::get<Interval>(std::get<Range>(trait.data).range).mesh.vertices;})
                | std::views::join | std::ranges::to<std::vector>();
                num_interval_vertices = vertices_interval.size();
                glBindBuffer(GL_ARRAY_BUFFER, vbo_interval);
                glBufferData(GL_ARRAY_BUFFER, vertices_interval.size() * sizeof(Point), vertices_interval.data(), GL_DYNAMIC_DRAW);

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
                glBufferData(GL_ARRAY_BUFFER, tri_vertices.size() * sizeof(Point), tri_vertices.data(), GL_DYNAMIC_DRAW);

                glBindBuffer(GL_ARRAY_BUFFER, vbo_polypoint);
                glBufferData(GL_ARRAY_BUFFER, pt_vertices.size() * sizeof(Point), pt_vertices.data(), GL_DYNAMIC_DRAW);

                num_range_tri_vertices = tri_vertices.size();
                num_range_pt_vertices = pt_vertices.size();
                break;
            }
            // Parallel coordinates case
            default: {
                // Draw point trait for >2d case
                if (!parallel_highlight_lines_vertices.empty()) {
                    glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_lines_highlight);
                    glBufferData(GL_ARRAY_BUFFER, parallel_highlight_lines_vertices.size() * sizeof(Point), parallel_highlight_lines_vertices.data(), GL_DYNAMIC_DRAW);
                }
              
                // Draw range trait for >2d case
                if (!parallel_region_lines_vertices.empty()) {
                    glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_lines_region);
                    glBufferData(GL_ARRAY_BUFFER, parallel_region_lines_vertices.size() * sizeof(Point), parallel_region_lines_vertices.data(), GL_DYNAMIC_DRAW);
                    glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_region_fill);
                    glBufferData(GL_ARRAY_BUFFER, parallel_region_fill_vertices.size() * sizeof(Point), parallel_region_fill_vertices.data(), GL_DYNAMIC_DRAW);
                }
            }
        }
    }
    
    void AttribRenderer::set_field_data(std::shared_ptr<VolumeData>& vol) {
        data = vol;
        descriptors.clear();
        clear_traits();
        clear_plot();
    }
    
    void AttribRenderer::set_attrib_space_axis(const std::vector<AxisDesc>& ds) {
        if (!data || ds.empty()) {
            throw std::runtime_error("set_attrib_axis() called with ds.size() == 0 || data == null");
        };
        
        descriptors.clear();
        clear_traits();
        clear_plot();
        for (auto& val: ds) {
            auto it = data->scalars.find(val.comp_name);
            if (it == data->scalars.end()) continue;
            auto& comp = it->second;
            auto [mn, mx] = std::minmax_element(comp.begin(), comp.end());
        #ifdef MVF_DEBUG
            std::cout << std::format("Field-{}: min_val={:.2f}, max_val={:.2f}", val.comp_name, *mn, *mx) << std::endl;
        #endif 
          
            descriptors.push_back(AxisDescMeta{.desc = val, .min_val = *mn, .max_val = *mx});
        }
        if (descriptors.size() == 1) generate_freq_distribution();
        else if (descriptors.size() == 2) generate_scatter_plot();
        else generate_parallel_coordinates();
    }
           
    void AttribRenderer::enable_plot(bool enable) {
        if (enable) {
            setup_plot();
        }
        is_plot_visible = enable;
    }

    std::pair<std::vector<AxisDescMeta>, std::vector<Trait>> AttribRenderer::get_traits() {
        return {descriptors, traits};
    }

    void AttribRenderer::clear_traits() {
        traits.clear();
        num_interval_vertices = 0;
        num_range_pt_vertices = 0;
        num_range_tri_vertices = 0;
        parallel_highlight_lines_vertices.clear();
        parallel_region_lines_vertices.clear();
        parallel_region_fill_vertices.clear();
        parallel_pending_marker_positions.clear();
    }

    void AttribRenderer::generate_parallel_coordinates() {
        if (descriptors.size() < 3 || !data) {
            throw std::runtime_error("generate_parallel_coordinates() called with descriptors.size() < 3");
        }
        parallel_axes_vertices.clear();
        parallel_lines_vertices.clear();

        constexpr size_t sample_period = 1000;

        float span = AXIS_LENGTH; 
        size_t n = descriptors.size();
        float dx = span / (n - 1);
        float x0 = -AXIS_LENGTH / 2; 

        for (size_t i = 0; i < n; ++i) {
            float x = x0 + i * dx;
            parallel_axes_vertices.push_back(Vector2f(x, -AXIS_LENGTH/2));
            parallel_axes_vertices.push_back(Vector2f(x, AXIS_LENGTH/2));
        }
        
        size_t total = data->scalars[descriptors[0].desc.comp_name].size();
       
        for (size_t idx = 0; idx < total; idx += sample_period) {
            for (size_t axis = 0; axis < n - 1; axis++) {
                auto &compA = data->scalars[descriptors[axis].desc.comp_name];
                auto &compB = data->scalars[descriptors[axis + 1].desc.comp_name];
                float valA = compA[idx];
                float valB = compB[idx];
                float normA = 2 * (valA - descriptors[axis].min_val) / (descriptors[axis].max_val - descriptors[axis].min_val) - 1;
                float normB = 2 * (valB - descriptors[axis + 1].min_val) / (descriptors[axis + 1].max_val - descriptors[axis + 1].min_val) - 1;
                float xA = x0 + axis * dx;
                float xB = x0 + (axis + 1) * dx;
                float yA = normA * (AXIS_LENGTH / 2);
                float yB = normB * (AXIS_LENGTH / 2);
                parallel_lines_vertices.push_back(Vector2f(xA, yA));
                parallel_lines_vertices.push_back(Vector2f(xB, yB));
            }
        }
        setup_parallel_coordinates();
    }

    void AttribRenderer::clear_parallel_pending_markers() {
        parallel_pending_marker_positions.clear();
    }

    void AttribRenderer::add_parallel_pending_marker(size_t axis_index, float world_y) {
        if (descriptors.size() < 3) {
            throw std::runtime_error("add_parallel_pending_marker() called with descriptors.size() < 3");
        }

        const auto pending_marker_color = Vector3f(0, 0, 0);
        float span = AXIS_LENGTH; size_t n = descriptors.size(); float dx = span / (n - 1); float x0 = -AXIS_LENGTH / 2;
        float x = x0 + axis_index * dx;
        parallel_pending_marker_positions.push_back(Point{x, world_y, pending_marker_color});
        setup_pending_markers();
    }

    void AttribRenderer::select_parallel_line_by_points(const std::vector<float>& axis_world_ys) {
        if (descriptors.size() < 3 || !data) {
            throw std::runtime_error("selected_parallel_line_by_points() called with descriptors.size() < 3");
        }

        size_t n = descriptors.size();
        if (axis_world_ys.size() != n) {
            throw std::runtime_error("select_parallel_line_by_points() called with axis_world_ys.size() != n");
        }

        // Append user-selected polyline segments; keep old highlights
        auto color_id = get_color_id();
        auto& color = global_color_pallete[color_id];
        float span = AXIS_LENGTH; float dx = span / (n - 1); float x0 = -AXIS_LENGTH / 2;
        for (size_t axis = 0; axis < n - 1; axis++) {
            float xA = x0 + axis * dx; 
            float xB = x0 + (axis + 1) * dx;
            parallel_highlight_lines_vertices.push_back(Point{xA, axis_world_ys[axis], color});
            parallel_highlight_lines_vertices.push_back(Point{xB, axis_world_ys[axis + 1], color});
        }

        clear_parallel_pending_markers();
        NDPoint nd {axis_world_ys};
        traits.push_back(Trait{.type = TraitType::PARALLEL_POINT, .data = nd, .color_id = color_id});
        setup_traits();
    }

    void AttribRenderer::select_parallel_region_by_ranges(const std::vector<std::pair<float,float>>& axis_world_ranges) {
        if (descriptors.size() < 3 || !data) {
            throw std::runtime_error("selected_parallel_region_by_ranges() called with descriptors.size() < 3");
        }

        size_t n = descriptors.size();
        if (axis_world_ranges.size() != n) return;
       
        float span = AXIS_LENGTH; float dx = span / (n - 1); float x0 = -AXIS_LENGTH / 2;
        auto color_id = get_color_id();
        auto& color = global_color_pallete[color_id];
        for (size_t axis = 0; axis < n - 1; axis++) {
            float xA = x0 + axis * dx; float xB = x0 + (axis + 1) * dx;
            auto [a0, a1] = axis_world_ranges[axis];
            auto [b0, b1] = axis_world_ranges[axis + 1];
            if (a0 > a1) std::swap(a0, a1);
            if (b0 > b1) std::swap(b0, b1);
            parallel_region_lines_vertices.push_back(Point{xA, a0, color}); parallel_region_lines_vertices.push_back(Point{xB, b0, color});
            parallel_region_lines_vertices.push_back(Point{xA, a1, color}); parallel_region_lines_vertices.push_back(Point{xB, b1, color});
            parallel_region_lines_vertices.push_back(Point{xA, a0, color}); parallel_region_lines_vertices.push_back(Point{xA, a1, color});
            parallel_region_lines_vertices.push_back(Point{xB, b0, color}); parallel_region_lines_vertices.push_back(Point{xB, b1, color});
            
            Point v0{xA, a0, color}, v1{xA, a1, color}, v2{xB, b0, color}, v3{xB, b1, color};
            
            parallel_region_fill_vertices.push_back(v0);
            parallel_region_fill_vertices.push_back(v2);
            parallel_region_fill_vertices.push_back(v1);
            parallel_region_fill_vertices.push_back(v1);
            parallel_region_fill_vertices.push_back(v2);
            parallel_region_fill_vertices.push_back(v3);
        }

        clear_parallel_pending_markers();
        Range r; r.type = RangeType::HYPERBOX; r.range = HyperBox{.yranges = axis_world_ranges};
        traits.push_back(Trait{.type = TraitType::RANGE, .data = r, .color_id = color_id});
        setup_traits();
    }

    void AttribRenderer::render() {
        glClear(GL_COLOR_BUFFER_BIT);

        if (descriptors.empty()) return;
        
        const Vector4f box_color(0, 0, 0, 1.0f);
        auto pipeline_axis = reinterpret_cast<AxisPipeline*>(pipelines[static_cast<int>(PipelineType::AXIS)]);
        glUseProgram(pipeline_axis->shader_program);
        glUniform4fv(pipeline_axis->uColor, 1, (float*)&box_color);

        if (descriptors.size() <= 2) {
            glBindVertexArray(vao_x_axis);
            glDrawArrays(GL_TRIANGLES, 0, axis_mesh_x.vertices.size());
        }

        if (descriptors.size() == 1 && is_plot_visible && dist_plot_solid.size()) {
            const Vector4f rect_color(1, 0, 1, 1);
            glUniform4fv(pipeline_axis->uColor, 1, (float*)&rect_color);
            glBindVertexArray(vao_distplotsolid);
            glDrawArrays(GL_TRIANGLES, 0, dist_plot_solid.size());
            
            const Vector4f line_color(0, 0, 0, 1);
            glUniform4fv(pipeline_axis->uColor, 1, (float*)&line_color);
            glBindVertexArray(vao_distplotlines);
            glDrawArrays(GL_LINES, 0, dist_plot_lines.size());
        }
        else if (descriptors.size() == 2) {
            glBindVertexArray(vao_y_axis); 
            glDrawArrays(GL_TRIANGLES, 0, axis_mesh_y.vertices.size());
            if (is_plot_visible && scatter_plot.size()) {
                const Vector4f point_color(1, 0, 1, 1);
                glUniform4fv(pipeline_axis->uColor, 1, (float*)&point_color);
                glBindVertexArray(vao_scatterplot);
                glDrawArrays(GL_POINTS, 0, scatter_plot.size());
            }
        }
        else if (descriptors.size() > 2) {
            const Vector4f axis_color(0, 0, 0, 1);
            glUniform4fv(pipeline_axis->uColor, 1, (float*)&axis_color);
            glBindVertexArray(vao_parallel_axes);
            glDrawArrays(GL_LINES, 0, parallel_axes_vertices.size());
            
            if (is_plot_visible) {
                if (!parallel_lines_vertices.empty()) {
                    const Vector4f line_color(1.0f, 0.0f, 0.0f, 0.35f);
                    glUniform4fv(pipeline_axis->uColor, 1, (float*)&line_color);
                    glBindVertexArray(vao_parallel_lines);
                    glDrawArrays(GL_LINES, 0, parallel_lines_vertices.size());
                }
            }
            
            // Draw any pending marker positions
            if (!parallel_pending_marker_positions.empty()) {
                auto pipeline_marker = reinterpret_cast<MarkerPipeline*>(pipelines[static_cast<int>(PipelineType::MARKER)]);
                glUseProgram(pipeline_marker->shader_program);
                glUniform1f(pipeline_marker->uAlpha, 1.0f);
                glBindVertexArray(vao_marker);
                glDrawArraysInstanced(GL_TRIANGLES, 0, marker.vertices.size(), parallel_pending_marker_positions.size());
            }
        }
        
        // Draw the traits
        auto point_traits = std::ranges::distance(traits | std::views::filter([] (auto& trait) {
            return trait.type == TraitType::POINT;
        }));

        if (point_traits) {
            auto pipeline_marker = reinterpret_cast<MarkerPipeline*>(pipelines[static_cast<int>(PipelineType::MARKER)]);
            glUseProgram(pipeline_marker->shader_program);
            glUniform1f(pipeline_marker->uAlpha, 1.0f);
            glBindVertexArray(vao_marker);
            glDrawArraysInstanced(GL_TRIANGLES, 0, marker.vertices.size(), point_traits);
        }
      
        // Draw 2d range traits
        auto pipeline_color = reinterpret_cast<ColorPipeline*>(pipelines[static_cast<int>(PipelineType::COLOR)]);
        glUseProgram(pipeline_color->shader_program);
        if (num_interval_vertices) {
            glUniform1f(pipeline_color->uAlpha, 0.5f);

            glBindVertexArray(vao_interval);
            glDrawArrays(GL_TRIANGLES, 0, num_interval_vertices);
        } else if (num_range_tri_vertices) {
            glUniform1f(pipeline_color->uAlpha, 0.5f);

            glBindVertexArray(vao_polyline);
            glDrawArrays(GL_TRIANGLES, 0, num_range_tri_vertices);

            glUniform1f(pipeline_color->uAlpha, 1.0f);
            glBindVertexArray(vao_polypoint);
            glDrawArrays(GL_POINTS, 0, num_range_pt_vertices);
        }
       
        // Draw parallel point traits
        auto parallel_traits = std::ranges::distance(traits | std::views::filter([] (auto& trait) {
            return trait.type == TraitType::PARALLEL_POINT;
        }));

        if (parallel_traits && !parallel_highlight_lines_vertices.empty()) {
            glUniform1f(pipeline_color->uAlpha, 1.0f);
            glBindVertexArray(vao_parallel_lines_highlight);
            glDrawArrays(GL_LINES, 0, parallel_highlight_lines_vertices.size());
        }
        
        if (!parallel_region_fill_vertices.empty()) {
            glUniform1f(pipeline_color->uAlpha, 0.5f);
            glBindVertexArray(vao_parallel_region_fill);
            glDrawArrays(GL_TRIANGLES, 0, parallel_region_fill_vertices.size());
        }
        if (!parallel_region_lines_vertices.empty()) {
            glUniform1f(pipeline_color->uAlpha, 1.0f);
            glBindVertexArray(vao_parallel_lines_region);
            glDrawArrays(GL_LINES, 0, parallel_region_lines_vertices.size());
        }

        glBindVertexArray(0);
    }

    void AttribRenderer::resync() { 
        setup_buffers(); 
    }

    void AttribRenderer::set_range_trait(float x1, float x2) {
        if (descriptors.size() != 1 || std::abs(x1) >= AXIS_LENGTH / 2 || std::abs(x2) >= AXIS_LENGTH / 2) return;
        traits.push_back(Trait {.type = TraitType::RANGE, .data = Range{.type = RangeType::INTERVAL, .range = Interval{.left = x1, .right = x2, 
            .mesh = IntervalSelector(x1, x2, 0, 0.01f, 0.03f, 0.1f, global_color_pallete[last_chosen_id])}}, .color_id = get_color_id()});
        setup_traits();
    }
    
    void AttribRenderer::modify_range_trait(float x_right) {
        if (traits.size() == 0) throw std::runtime_error("Called modify_range_trait() with traits.size() == 0");
        auto& last_trait = traits.back();
        if (last_trait.type != TraitType::RANGE || std::get<Range>(last_trait.data).type != RangeType::INTERVAL) throw std::runtime_error("Called modify_range_trait() on interval trait, but last added trait was not interval...");
        if (descriptors.size() != 1 || std::abs(x_right) >= AXIS_LENGTH / 2) return;
        auto saved_x_left = std::get<Interval>(std::get<Range>(last_trait.data).range).left;  
        std::get<Interval>(std::get<Range>(last_trait.data).range).right = x_right;
        std::get<Interval>(std::get<Range>(last_trait.data).range).mesh = IntervalSelector(saved_x_left, x_right, 0, 0.01f, 
            0.03f, 0.1f, global_color_pallete[last_trait.color_id]);
        
        setup_traits();
    }
    
    void AttribRenderer::set_range_trait(float x_top, float y_top, float width, float height) {
        if (descriptors.size() != 2 || std::abs(x_top) >= AXIS_LENGTH / 2 || std::abs(y_top) >= AXIS_LENGTH / 2 
            || std::abs(x_top + width) >= AXIS_LENGTH / 2 || std::abs(y_top + height) >= AXIS_LENGTH / 2) return;
        traits.push_back(Trait {.type = TraitType::RANGE, .data = Range{.type = RangeType::POLYGON, .range = Polygon{.x_top = x_top,
            .y_top = y_top, .width = width, .height = height, .mesh = PolySelector(x_top, y_top, width, height, global_color_pallete[last_chosen_id])}}, .color_id = get_color_id()});

        setup_traits();
    }

    void AttribRenderer::modify_range_trait(float x_end, float y_end) {
        if (traits.size() == 0) throw std::runtime_error("Called modify_range_trait() with traits.size() == 0");
        auto& last_trait = traits.back();
        if (last_trait.type != TraitType::RANGE || std::get<Range>(last_trait.data).type != RangeType::POLYGON) throw std::runtime_error("Called modify_range_trait() on polygon trait, but last added trait was not polygon...");
        if (descriptors.size() != 2 || std::abs(x_end) >= AXIS_LENGTH / 2 || std::abs(y_end) >= AXIS_LENGTH / 2) return;
        auto saved_x_top = std::get<Polygon>(std::get<Range>(last_trait.data).range).x_top;
        auto saved_y_top = std::get<Polygon>(std::get<Range>(last_trait.data).range).y_top;
        auto width = x_end - saved_x_top;
        auto height = y_end - saved_y_top; 
        std::get<Polygon>(std::get<Range>(last_trait.data).range).width = width;
        std::get<Polygon>(std::get<Range>(last_trait.data).range).height = height;
        std::get<Polygon>(std::get<Range>(last_trait.data).range).mesh = PolySelector(saved_x_top, saved_y_top, 
            width, height, global_color_pallete[last_trait.color_id]);
        setup_traits();
    }
    
    void AttribRenderer::set_point_trait(float x) { 
        set_point_trait(x, 0);
    }

    void AttribRenderer::set_point_trait(float x, float y) {
        if (descriptors.size() < 1 || std::abs(x) >= AXIS_LENGTH / 2 || std::abs(y) >= AXIS_LENGTH / 2) return;
        traits.push_back(Trait {.type = TraitType::POINT, .data = Point{.x = x, .y = y, .color = global_color_pallete[last_chosen_id]}, .color_id = get_color_id()});
        setup_traits();
    }
    
    float AttribRenderer::get_field_point(float t, size_t id) {
        if (descriptors.size() <= id) throw std::runtime_error(std::format("get_field_point() called with id {} when descriptors.size() = {}", id, descriptors.size()));
        auto& desc = descriptors[id];
        auto u_norm = t / (AXIS_LENGTH / 2);
        return 0.5f * ((desc.max_val + desc.min_val) + u_norm * (desc.max_val - desc.min_val));
    }

    void AttribRenderer::clear_plot() {
        scatter_plot.clear();
        dist_plot_solid.clear();
        dist_plot_lines.clear();
        parallel_axes_vertices.clear();
        parallel_lines_vertices.clear();
        parallel_highlight_lines_vertices.clear();
        parallel_region_lines_vertices.clear();
        parallel_region_fill_vertices.clear();
        parallel_pending_marker_positions.clear();
        is_plot_visible = false;
    }
}
