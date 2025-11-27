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
        glGenVertexArrays(1, &vao_parallel_marker);

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
        glGenBuffers(1, &vbo_parallel_marker_pos);
        
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
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (void*)0);
        glBindVertexArray(vao_parallel_lines_region);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_lines_region);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (void*)0);
        glBindVertexArray(vao_parallel_region_fill);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_region_fill);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (void*)0);

        glBindVertexArray(vao_parallel_marker);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_marker);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_marker_pos);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), 0);
        glVertexAttribDivisor(1, 1);

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
        else if (parallel_lines_vertices.size()) {
            glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_axes);
            glBufferData(GL_ARRAY_BUFFER, parallel_axes_vertices.size() * sizeof(Vector2f), parallel_axes_vertices.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_lines);
            glBufferData(GL_ARRAY_BUFFER, parallel_lines_vertices.size() * sizeof(Vector2f), parallel_lines_vertices.data(), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_lines_highlight);
            glBufferData(GL_ARRAY_BUFFER, parallel_highlight_lines_vertices.size() * sizeof(Vector2f), parallel_highlight_lines_vertices.data(), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_lines_region);
            glBufferData(GL_ARRAY_BUFFER, parallel_region_lines_vertices.size() * sizeof(Vector2f), parallel_region_lines_vertices.data(), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_region_fill);
            glBufferData(GL_ARRAY_BUFFER, parallel_region_fill_vertices.size() * sizeof(Vector2f), parallel_region_fill_vertices.data(), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_marker_pos);
            glBufferData(GL_ARRAY_BUFFER, parallel_pending_marker_positions.size() * sizeof(Vector2f), parallel_pending_marker_positions.data(), GL_DYNAMIC_DRAW);
        }
    }
    
    void AttribRenderer::setup_traits() {
        if (!traits.size()) {
            return;
        }
        if (!descriptors.size()) {
            throw std::runtime_error("setup_traits() called when descriptors.size() == 0");
        }
        auto vertices_point = traits | std::views::filter([] (auto& trait) {
            return trait.type == TraitType::POINT;
        }) | std::views::transform([] (auto& trait) {return std::get<Point>(trait.data);} )
        | std::ranges::to<std::vector>();
        glBindBuffer(GL_ARRAY_BUFFER, vbo_marker_pos);
        glBufferData(GL_ARRAY_BUFFER, vertices_point.size() * sizeof(Point), vertices_point.data(), GL_DYNAMIC_DRAW);
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
    
    void AttribRenderer::set_attrib_space_axis(const std::vector<AxisDesc>& ds) {
        if (!data) return;
        descriptors.clear();
        clear_traits();
        clear_plot();
        for (auto& val: ds) {
            auto it = data->scalars.find(val.comp_name);
            if (it == data->scalars.end()) continue;
            auto& comp = std::get<0>(it->second);
            auto [mn, mx] = std::minmax_element(comp.begin(), comp.end());
            descriptors.push_back(AxisDescMeta{.desc = val, .min_val = *mn, .max_val = *mx});
        }
        if (descriptors.size() == 1) generate_freq_distribution();
        else if (descriptors.size() == 2) generate_scatter_plot();
    }
           
    void AttribRenderer::enable_plot(bool enable) {
        if (enable) {
            if (descriptors.size() > 2) {
                generate_parallel_coordinates();
            }
            setup_plot();
        }
        is_plot_visible = enable;
    }

    std::pair<std::vector<AxisDescMeta>, std::vector<Trait>> AttribRenderer::get_traits() {
        return {descriptors, traits};
    }

    void AttribRenderer::set_sample_period(size_t period) {
        if (period == 0) period = 1;
        sample_period = period;
        if (is_plot_visible && descriptors.size() > 2) {
            generate_parallel_coordinates();
            setup_plot();
        }
    }

    void AttribRenderer::clear_traits() {
        traits.clear();
        num_interval_vertices = 0;
        num_range_pt_vertices = 0;
        num_range_tri_vertices = 0;
        // clear parallel selections
        parallel_highlight_lines_vertices.clear();
        parallel_region_lines_vertices.clear();
        parallel_region_fill_vertices.clear();
        parallel_pending_marker_positions.clear();
        has_pending_markers = false;
    }

    void AttribRenderer::cycle_sample_period() {
        static const size_t modes[] = {1, 100, 500, 1000};
        auto it = std::find(std::begin(modes), std::end(modes), sample_period);
        if (it == std::end(modes)) {
            sample_period = modes[0];
        } else {
            size_t idx = (std::distance(std::begin(modes), it) + 1) % (sizeof(modes)/sizeof(modes[0]));
            sample_period = modes[idx];
        }
        if (is_plot_visible && descriptors.size() > 2) {
            generate_parallel_coordinates();
            setup_plot();
        }
    }

    void AttribRenderer::generate_parallel_coordinates() {
        if (descriptors.size() < 3 || !data) {
            parallel_axes_vertices.clear();
            parallel_lines_vertices.clear();
            return;
        }
        parallel_axes_vertices.clear();
        parallel_lines_vertices.clear();
        float span = AXIS_LENGTH; 
        size_t n = descriptors.size();
        float dx = span / (n - 1);
        float x0 = -AXIS_LENGTH / 2; 
        for (size_t i = 0; i < n; ++i) {
            float x = x0 + i * dx;
            parallel_axes_vertices.push_back(Vector2f(x, -AXIS_LENGTH/2));
            parallel_axes_vertices.push_back(Vector2f(x, AXIS_LENGTH/2));
        }
        size_t total = std::get<0>(data->scalars[descriptors[0].desc.comp_name]).size();
        if (total == 0) return;
        for (size_t idx = 0; idx < total; idx += sample_period) {
            for (size_t axis = 0; axis + 1 < n; ++axis) {
                auto &compA = std::get<0>(data->scalars[descriptors[axis].desc.comp_name]);
                auto &compB = std::get<0>(data->scalars[descriptors[axis+1].desc.comp_name]);
                float valA = compA[idx];
                float valB = compB[idx];
                float normA = 2 * (valA - descriptors[axis].min_val) / (descriptors[axis].max_val - descriptors[axis].min_val) - 1;
                float normB = 2 * (valB - descriptors[axis+1].min_val) / (descriptors[axis+1].max_val - descriptors[axis+1].min_val) - 1;
                float xA = x0 + axis * dx;
                float xB = x0 + (axis+1) * dx;
                float yA = std::clamp(normA, -1.0f, 1.0f) * (AXIS_LENGTH/2);
                float yB = std::clamp(normB, -1.0f, 1.0f) * (AXIS_LENGTH/2);
                parallel_lines_vertices.push_back(Vector2f(xA, yA));
                parallel_lines_vertices.push_back(Vector2f(xB, yB));
            }
        }
    }

    // pending markers helpers
    void AttribRenderer::clear_parallel_pending_markers() {
        parallel_pending_marker_positions.clear();
        has_pending_markers = false;
        glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_marker_pos);
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    }
    void AttribRenderer::add_parallel_pending_marker(size_t axis_index, float world_y) {
        if (descriptors.size() < 3) return;
        float span = AXIS_LENGTH; size_t n = descriptors.size(); float dx = span / (n - 1); float x0 = -AXIS_LENGTH / 2;
        float x = x0 + axis_index * dx;
        parallel_pending_marker_positions.push_back(Vector2f(x, world_y));
        has_pending_markers = true;
        glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_marker_pos);
        glBufferData(GL_ARRAY_BUFFER, parallel_pending_marker_positions.size() * sizeof(Vector2f), parallel_pending_marker_positions.data(), GL_DYNAMIC_DRAW);
    }

    // --- Parallel selections ---
    void AttribRenderer::select_parallel_line_by_points(const std::vector<float>& axis_world_ys) {
        if (descriptors.size() < 3 || !data) return;
        size_t n = descriptors.size();
        if (axis_world_ys.size() != n) return;
        // append user-selected polyline segments; keep old highlights
        float span = AXIS_LENGTH; size_t nAx = n; float dx = span / (nAx - 1); float x0 = -AXIS_LENGTH / 2;
        for (size_t axis = 0; axis + 1 < nAx; ++axis) {
            float xA = x0 + axis * dx; float xB = x0 + (axis+1) * dx;
            float yA = std::clamp(axis_world_ys[axis] / (AXIS_LENGTH/2.0f), -1.0f, 1.0f) * (AXIS_LENGTH/2.0f);
            float yB = std::clamp(axis_world_ys[axis+1] / (AXIS_LENGTH/2.0f), -1.0f, 1.0f) * (AXIS_LENGTH/2.0f);
            parallel_highlight_lines_vertices.push_back(Vector2f(xA, yA));
            parallel_highlight_lines_vertices.push_back(Vector2f(xB, yB));
        }
        glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_lines_highlight);
        glBufferData(GL_ARRAY_BUFFER, parallel_highlight_lines_vertices.size() * sizeof(Vector2f), parallel_highlight_lines_vertices.data(), GL_DYNAMIC_DRAW);
        clear_parallel_pending_markers();
        NDPoint nd; nd.ys = axis_world_ys;
        traits.push_back(Trait{.type = TraitType::PARALLEL_POINT, .data = nd});
    }

    void AttribRenderer::select_parallel_region_by_ranges(const std::vector<std::pair<float,float>>& axis_world_ranges) {
        if (descriptors.size() < 3 || !data) return;
        size_t n = descriptors.size(); if (axis_world_ranges.size() != n) return;
        parallel_region_lines_vertices.clear();
        parallel_region_fill_vertices.clear();
        float span = AXIS_LENGTH; float dx = span / (n - 1); float x0 = -AXIS_LENGTH / 2;
        for (size_t axis = 0; axis + 1 < n; ++axis) {
            float xA = x0 + axis * dx; float xB = x0 + (axis+1) * dx;
            auto [a0, a1] = axis_world_ranges[axis];
            auto [b0, b1] = axis_world_ranges[axis+1];
            if (a0 > a1) std::swap(a0, a1); if (b0 > b1) std::swap(b0, b1);
            parallel_region_lines_vertices.push_back(Vector2f(xA, a0)); parallel_region_lines_vertices.push_back(Vector2f(xB, b0));
            parallel_region_lines_vertices.push_back(Vector2f(xA, a1)); parallel_region_lines_vertices.push_back(Vector2f(xB, b1));
            parallel_region_lines_vertices.push_back(Vector2f(xA, a0)); parallel_region_lines_vertices.push_back(Vector2f(xA, a1));
            parallel_region_lines_vertices.push_back(Vector2f(xB, b0)); parallel_region_lines_vertices.push_back(Vector2f(xB, b1));
            Vector2f v0(xA, a0), v1(xA, a1), v2(xB, b0), v3(xB, b1);
            parallel_region_fill_vertices.push_back(v0);
            parallel_region_fill_vertices.push_back(v2);
            parallel_region_fill_vertices.push_back(v1);
            parallel_region_fill_vertices.push_back(v1);
            parallel_region_fill_vertices.push_back(v2);
            parallel_region_fill_vertices.push_back(v3);
        }
        glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_lines_region);
        glBufferData(GL_ARRAY_BUFFER, parallel_region_lines_vertices.size() * sizeof(Vector2f), parallel_region_lines_vertices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_parallel_region_fill);
        glBufferData(GL_ARRAY_BUFFER, parallel_region_fill_vertices.size() * sizeof(Vector2f), parallel_region_fill_vertices.data(), GL_DYNAMIC_DRAW);
        clear_parallel_pending_markers();
        Range r; r.type = RangeType::HYPERBOX; r.range = HyperBox{.yranges = axis_world_ranges};
        traits.push_back(Trait{.type = TraitType::RANGE, .data = r});
    }

    void AttribRenderer::render() {
        glClear(GL_COLOR_BUFFER_BIT);
        if (descriptors.empty()) return;
        const Vector4f box_color(0, 0, 0, 1.0f);
        auto pipeline_axis = reinterpret_cast<AxisPipeline*>(pipelines[static_cast<int>(PipelineType::AXIS)]);
        glUseProgram(pipeline_axis->shader_program);
        glUniform4fv(pipeline_axis->uColor, 1, (float*)&box_color);
        if (descriptors.size() == 1 || descriptors.size() == 2) {
            glBindVertexArray(vao_x_axis);
            glDrawArrays(GL_TRIANGLES, 0, axis_mesh_x.vertices.size());
        }
        if (descriptors.size() == 1 && is_plot_visible && dist_plot_solid.size()) {
            const Vector4f rect_color(1, 0, 1, 1); glUniform4fv(pipeline_axis->uColor, 1, (float*)&rect_color);
            glBindVertexArray(vao_distplotsolid); glDrawArrays(GL_TRIANGLES, 0, dist_plot_solid.size());
            const Vector4f line_color(0, 0, 0, 1); glUniform4fv(pipeline_axis->uColor, 1, (float*)&line_color);
            glBindVertexArray(vao_distplotlines); glDrawArrays(GL_LINES, 0, dist_plot_lines.size());
        }
        else if (descriptors.size() == 2) {
            glBindVertexArray(vao_y_axis); glDrawArrays(GL_TRIANGLES, 0, axis_mesh_y.vertices.size());
            if (is_plot_visible && scatter_plot.size()) {
                const Vector4f point_color(1, 0, 1, 1); glUniform4fv(pipeline_axis->uColor, 1, (float*)&point_color);
                glBindVertexArray(vao_scatterplot); glDrawArrays(GL_POINTS, 0, scatter_plot.size());
            }
        }
        else if (descriptors.size() > 2 && is_plot_visible) {
            if (!parallel_axes_vertices.empty()) {
                const Vector4f axis_color(0, 0, 0, 1); glUniform4fv(pipeline_axis->uColor, 1, (float*)&axis_color);
                glBindVertexArray(vao_parallel_axes);
                glDrawArrays(GL_LINES, 0, parallel_axes_vertices.size());
            }
            if (!parallel_lines_vertices.empty()) {
                const Vector4f line_color(1.0f, 0.0f, 0.0f, 0.35f);
                glUniform4fv(pipeline_axis->uColor, 1, (float*)&line_color);
                glBindVertexArray(vao_parallel_lines);
                glDrawArrays(GL_LINES, 0, parallel_lines_vertices.size());
            }
            if (!parallel_region_fill_vertices.empty()) {
                const Vector4f fill_color(1.0f, 1.0f, 0.0f, 0.2f);
                glUniform4fv(pipeline_axis->uColor, 1, (float*)&fill_color);
                glBindVertexArray(vao_parallel_region_fill);
                glDrawArrays(GL_TRIANGLES, 0, parallel_region_fill_vertices.size());
            }
            if (!parallel_region_lines_vertices.empty()) {
                const Vector4f region_color(1.0f, 0.9f, 0.0f, 0.9f);
                glUniform4fv(pipeline_axis->uColor, 1, (float*)&region_color);
                glBindVertexArray(vao_parallel_lines_region);
                glDrawArrays(GL_LINES, 0, parallel_region_lines_vertices.size());
            }
            if (!parallel_highlight_lines_vertices.empty()) {
                const Vector4f sel_color(0.1f, 0.3f, 1.0f, 1.0f);
                glUniform4fv(pipeline_axis->uColor, 1, (float*)&sel_color);
                glBindVertexArray(vao_parallel_lines_highlight);
                glDrawArrays(GL_LINES, 0, parallel_highlight_lines_vertices.size());
            }
            if (has_pending_markers && !parallel_pending_marker_positions.empty()) {
                const Vector4f marker_color(1, 0, 0, 1);
                auto pipeline_marker = reinterpret_cast<MarkerPipeline*>(pipelines[static_cast<int>(PipelineType::MARKER)]);
                glUseProgram(pipeline_marker->shader_program);
                glUniform4fv(pipeline_marker->uColor, 1, (float*)&marker_color);
                glBindVertexArray(vao_parallel_marker);
                glDrawArraysInstanced(GL_TRIANGLES, 0, marker.vertices.size(), static_cast<GLsizei>(parallel_pending_marker_positions.size()));
            }
        }
        if (descriptors.size() <= 2) {
            auto point_traits = std::ranges::distance(traits | std::views::filter([](auto& tr) {return tr.type == TraitType::POINT; }));
            if (point_traits) {
                const Vector4f marker_color(1, 0, 0, 1); auto pipeline_marker = reinterpret_cast<MarkerPipeline*>(pipelines[static_cast<int>(PipelineType::MARKER)]);
                glUseProgram(pipeline_marker->shader_program); glUniform4fv(pipeline_marker->uColor, 1, (float*)&marker_color);
                glBindVertexArray(vao_marker); glDrawArraysInstanced(GL_TRIANGLES, 0, marker.vertices.size(), point_traits);
            }
            if (num_interval_vertices) {
                const Vector4f interval_color(1.0f, 0.8f, 0, 0.8f); glUniform4fv(pipeline_axis->uColor, 1, (float*)&interval_color); glBindVertexArray(vao_interval); glDrawArrays(GL_TRIANGLES, 0, num_interval_vertices);
            }
            else if (num_range_tri_vertices) {
                Vector4f line_color(1.0f, 0.8f, 0, 0.8f); glUniform4fv(pipeline_axis->uColor, 1, (float*)&line_color); glBindVertexArray(vao_polyline); glDrawArrays(GL_TRIANGLES, 0, num_range_tri_vertices);
                line_color = Vector4f(1.0f, 0.2f, 0, 1.0f); glUniform4fv(pipeline_axis->uColor, 1, (float*)&line_color); glBindVertexArray(vao_polypoint); glDrawArrays(GL_POINTS, 0, num_range_pt_vertices);
            }
        }
        glBindVertexArray(0);
    }

    void AttribRenderer::resync() { setup_buffers(); }

    void AttribRenderer::set_range_trait(float x1, float x2) {
        if (descriptors.size() != 1 || std::abs(x1) >= AXIS_LENGTH / 2 || std::abs(x2) >= AXIS_LENGTH / 2) return;
        traits.push_back(Trait {.type = TraitType::RANGE, .data = Range{.type = RangeType::INTERVAL, .range = Interval{.left = x1, .right = x2, 
            .mesh = IntervalSelector(x1, x2, 0, 0.01f, 0.03f, 0.1f)}}});
        setup_traits();
    }
    
    void AttribRenderer::modify_range_trait(float x_right) {
        if (traits.size() == 0) throw std::runtime_error("Called modify_range_trait() with traits.size() == 0");
        auto& last_trait = traits.back();
        if (last_trait.type != TraitType::RANGE || std::get<Range>(last_trait.data).type != RangeType::INTERVAL) throw std::runtime_error("Called modify_range_trait() on interval trait, but last added trait was not interval...");
        if (descriptors.size() != 1 || std::abs(x_right) >= AXIS_LENGTH / 2) return;
        auto saved_x_left = std::get<Interval>(std::get<Range>(last_trait.data).range).left;  
        std::get<Interval>(std::get<Range>(last_trait.data).range).right = x_right;
        std::get<Interval>(std::get<Range>(last_trait.data).range).mesh = IntervalSelector(saved_x_left, x_right, 0, 0.01f, 0.03f, 0.1f);
        setup_traits();
    }
    
    void AttribRenderer::set_range_trait(float x_top, float y_top, float width, float height) {
        if (descriptors.size() != 2 || std::abs(x_top) >= AXIS_LENGTH / 2 || std::abs(y_top) >= AXIS_LENGTH / 2 
            || std::abs(x_top + width) >= AXIS_LENGTH / 2 || std::abs(y_top + height) >= AXIS_LENGTH / 2) return;
        traits.push_back(Trait {.type = TraitType::RANGE, .data = Range{.type = RangeType::POLYGON, .range = Polygon{.x_top = x_top,
            .y_top = y_top, .width = width, .height = height, .mesh = PolySelector(x_top, y_top, width, height)}}});
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
        std::get<Polygon>(std::get<Range>(last_trait.data).range).mesh = PolySelector(saved_x_top, saved_y_top, width, height);
        setup_traits();
    }
    
    void AttribRenderer::set_point_trait(float x) { set_point_trait(x, 0); }

    void AttribRenderer::set_point_trait(float x, float y) {
        if (descriptors.size() < 1 || std::abs(x) >= AXIS_LENGTH / 2 || std::abs(y) >= AXIS_LENGTH / 2) return;
        traits.push_back(Trait {.type = TraitType::POINT, .data = Point{.x = x, .y = y}});
        setup_traits();
    }
    
    float AttribRenderer::get_field_point(float t, size_t id) {
        if (descriptors.size() <= id) throw std::runtime_error(std::format("get_field_point() called with id {} when descriptors.size() = {}", id, descriptors.size()));
        auto& desc = descriptors[id];
        auto u_norm = t / (AXIS_LENGTH / 2);
        return 0.5f * ((desc.max_val + desc.min_val) + u_norm * (desc.max_val - desc.min_val));
    }

    std::pair<float, float> AttribRenderer::screen_to_world(double sx, double sy) {
        float ndc_x = (float)(sx / viewport_width) * 2.0f - 1.0f;
        float ndc_y = 1.0f - (float)(sy / viewport_height) * 2.0f;
        float wx = ndc_x * (AXIS_LENGTH / 2.0f);
        float wy = ndc_y * (AXIS_LENGTH / 2.0f);
        return { wx, wy };
    }

    std::pair<float, float> AttribRenderer::world_to_screen(float wx, float wy) {
        float ndc_x = wx / (AXIS_LENGTH / 2.0f);
        float ndc_y = wy / (AXIS_LENGTH / 2.0f);
        float sx = (ndc_x * 0.5f + 0.5f) * viewport_width;
        float sy = (0.5f - ndc_y * 0.5f) * viewport_height;
        return { sx, sy };
    }

    void AttribRenderer::on_mouse_move(double mx, double my) {
        auto [w_x, w_y] = screen_to_world(mx, my);
        hovered_axis = -1;
        hovered_world_y = 0.0f;
        hover_text.clear();
        if (descriptors.size() < 3) return;
        size_t n = descriptors.size();
        float span = AXIS_LENGTH;
        float dx = span / (n - 1);
        float x0 = -AXIS_LENGTH / 2.0f;
        constexpr float HIT_THRESHOLD = 0.03f * AXIS_LENGTH;
        for (size_t i = 0; i < n; ++i) {
            float axis_x = x0 + i * dx;
            if (std::abs(w_x - axis_x) <= HIT_THRESHOLD) {
                hovered_axis = static_cast<int>(i);
                hovered_world_y = w_y;
                auto& meta = descriptors[i];
                float unorm = w_y / (AXIS_LENGTH / 2.0f);
                unorm = std::clamp(unorm, -1.0f, 1.0f);
                float real_val = 0.5f * ((meta.max_val + meta.min_val) + unorm * (meta.max_val - meta.min_val));
                hover_text = std::format("{} = {:.6g}", meta.desc.comp_name, real_val);
                return;
            }
        }
    }

    void AttribRenderer::draw_overlay_cairo(cairo_t* cr) {
        if (descriptors.size() < 3) return;
        PangoLayout* layout = pango_cairo_create_layout(cr);
        PangoFontDescription* fd = pango_font_description_from_string("Sans 11");
        pango_layout_set_font_description(layout, fd);
        pango_font_description_free(fd);
        size_t n = descriptors.size();
        float span = AXIS_LENGTH;
        float dx = span / (n - 1);
        float x0 = -AXIS_LENGTH / 2.0f;
        for (size_t i = 0; i < n; ++i) {
            float wx = x0 + i * dx;
            float wy = AXIS_LENGTH / 2.0f;
            auto [sx, sy] = world_to_screen(wx, wy);
            std::string name = descriptors[i].desc.comp_name;
            pango_layout_set_text(layout, name.c_str(), -1);
            int px, py; pango_layout_get_pixel_size(layout, &px, &py);
            cairo_move_to(cr, sx - px * 0.5, sy - py - 6);
            cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.85);
            cairo_rectangle(cr, sx - px * 0.5 - 4, sy - py - 8, px + 8, py + 6);
            cairo_fill(cr);
            cairo_set_source_rgb(cr, 0.05, 0.05, 0.05);
            pango_cairo_show_layout(cr, layout);
        }
        if (hovered_axis >= 0 && !hover_text.empty()) {
            float axis_x_world = x0 + hovered_axis * dx;
            auto [sx, sy] = world_to_screen(axis_x_world, hovered_world_y);
            pango_layout_set_text(layout, hover_text.c_str(), -1);
            int px, py; pango_layout_get_pixel_size(layout, &px, &py);
            const float margin = 6.0f;
            float box_x = sx + margin; float box_y = sy - py * 0.5f;
            cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.9);
            cairo_rectangle(cr, box_x - 4, box_y - 4, px + 8, py + 8);
            cairo_fill(cr);
            cairo_set_source_rgb(cr, 0.05, 0.05, 0.05);
            cairo_move_to(cr, box_x, box_y + 2);
            pango_cairo_show_layout(cr, layout);
        }
        g_object_unref(layout);
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
        has_pending_markers = false;
        is_plot_visible = false;
    }
}
