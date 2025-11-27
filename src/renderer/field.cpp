#include <ranges>
#include "renderer.h"
#include "entity.h"
#include "marching_cubes.h"
#include "attrib.h"
#include "ui_async.h"

namespace MVF {
    FieldEntity::FieldEntity() : Entity::Entity(Vector3f(0, 0, 0)) {
        create_voxel_grid();
    }

    void FieldEntity::init(VolumeEntity* geometry_entity) {
        this->geometry_entity = geometry_entity; 
        create_buffers();
        if (traits.size()) {
            build_texture();
        }
    }

    void FieldEntity::create_buffers() {
        // Create the 3d textures
        glGenTextures(1, &tex3d);
        glGenTextures(1, &tex3d_col);
        glBindTexture(GL_TEXTURE_3D, tex3d);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        
        glBindTexture(GL_TEXTURE_3D, tex3d_col);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
        size_t grid_size = res_x * res_y * res_z; 
        GLuint ssbo;
        glGenBuffers(1, &ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(tri_table), tri_table, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo); 
        
        GLuint ssbo_edge;
        glGenBuffers(1, &ssbo_edge);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_edge);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(edge_table), edge_table, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_edge); 
       
        // Create the buffers for isosurface extraction
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, grid_size * sizeof(Vertex), points.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void FieldEntity::create_voxel_grid() {
        auto x_step = 1.0f / res_x; 
        auto y_step = 1.0f / res_y; 
        auto z_step = 1.0f / res_z; 

        steps = Vector3f(x_step, y_step, z_step);

        size_t grid_size = res_x * res_y * res_z; 
        points.resize(grid_size);
        for (size_t i = 0; i < res_z; i++) {
            for (size_t j = 0; j < res_y; j++) {
                for (size_t k = 0; k < res_x; k++) {
                    auto x = k * x_step;
                    auto y = j * y_step;
                    auto z = i * z_step;
                    points[i * res_y * res_x + j * res_x + k] = Vertex {.x = x, .y = y, .z = z};
                }
            }
        }    
    }

    void FieldEntity::build_distance_field() {
        auto grid_size = geometry_entity->model->nx * geometry_entity->model->ny * geometry_entity->model->nz;
        field.resize(grid_size);
        color_field.resize(grid_size);
        
        float max_dist = 0;
        for (int i = 0; i < grid_size; i++) {
            float min_dist = INFINITY;
            if (stop_requested.load(std::memory_order_relaxed)) {
                compute_passed = false;
                return;
            }
           
            advance_ui_clock(static_cast<float>(i) / grid_size, false);
            // Get the point in attribute space for this point in domain
            std::vector<float> pt(attrib_comps.size());
            for (size_t dim = 0; dim < attrib_comps.size(); dim++) {
                auto& fld = std::get<0>(geometry_entity->model->scalars[attrib_comps[dim].desc.comp_name]);
                pt[dim] = fld[i];
            }

            // Calculate the min distance of the traits to this point
            // For now, we do this naively
            Trait* sel_trait = nullptr;
            for (auto& trait: traits) {
                float dist = INFINITY;
                switch(trait.type) {
                    case TraitType::POINT: {
                        auto& tr_pt = std::get<Point>(trait.data);
                        
                        // Calculate euclidean dist
                        // We won't take the square root, as we just need a varying field
                        if (attrib_comps.size() == 1) {
                            dist = (tr_pt.x - pt[0]) * (tr_pt.x - pt[0]);
                        } 
                        else if (attrib_comps.size() >= 2) {
                            dist = (tr_pt.y - pt[1]) * (tr_pt.y - pt[1]) + (tr_pt.x - pt[0]) * (tr_pt.x - pt[0]);
                        } 
                        break;
                    }
                    case TraitType::PARALLEL_POINT: {
                        auto& nd = std::get<NDPoint>(trait.data);
                        float dsum = 0.0f;
                        size_t m = std::min(nd.ys.size(), pt.size());
                        for (size_t a = 0; a < m; ++a) {
                            float d = nd.ys[a] - pt[a];
                            dsum += d * d;
                        }
                        dist = dsum;
                        break;
                    }
                    case TraitType::RANGE: {
                        auto& r = std::get<Range>(trait.data);
                        if (r.type == RangeType::INTERVAL) {
                            auto& tr_int = std::get<Interval>(r.range);
                            if (pt[0] >= tr_int.left && pt[0] <= tr_int.right) {
                                dist = 0;
                            }
                            else {
                                float d = std::min(std::abs(tr_int.left - pt[0]), std::abs(tr_int.right - pt[0]));
                                dist = d * d;
                            }
                        }
                        else if (r.type == RangeType::POLYGON) {
                            auto& tr_poly = std::get<Polygon>(r.range);
                            if (pt[0] >= tr_poly.x_top && pt[0] <= tr_poly.x_top + tr_poly.width 
                            && pt[1] >= tr_poly.y_top && pt[1] <= tr_poly.y_top + tr_poly.height) {
                                dist = 0;
                            }
                            else {
                                float mid_pt_x = tr_poly.x_top + tr_poly.width / 2;   
                                float mid_pt_y = tr_poly.y_top + tr_poly.height / 2;   
                                dist = (mid_pt_y - pt[1]) * (mid_pt_y - pt[1]) + (mid_pt_x - pt[0]) * (mid_pt_x - pt[0]);
                            }
                        } else if (r.type == RangeType::HYPERBOX) {
                            auto& hb = std::get<HyperBox>(r.range);
                            float dsum = 0.0f;
                            size_t m = std::min(hb.yranges.size(), pt.size());
                            for (size_t a = 0; a < m; ++a) {
                                float a0 = hb.yranges[a].first;
                                float a1 = hb.yranges[a].second;
                                if (a0 > a1) std::swap(a0, a1);
                                if (pt[a] < a0) { float d = a0 - pt[a]; dsum += d * d; }
                                else if (pt[a] > a1) { float d = pt[a] - a1; dsum += d * d; }
                                else { /* inside -> zero */ }
                            }
                            dist = dsum;
                        }
                    }
                }
               
                if (dist < min_dist) {
                    min_dist = dist;
                    sel_trait = &trait; 
                }
            }

            max_dist = std::max(min_dist, max_dist);
            field[i] = min_dist;
            color_field[i] = global_color_pallete[sel_trait->color_id];
        }

        // Normalize the field
        field = field | 
        std::views::transform([max_dist] (auto& pt) {
            return pt / max_dist;
        }) | std::ranges::to<std::vector<float>>();

#ifdef MVF_DEBUG
        size_t zero_count = 0;
        for (auto& val: field) {
            if (val >= 0 && val <= 0.05) {
                zero_count++;
            }
        }

        std::cout << "Zero count: " << zero_count << std::endl;
#endif

        field = field | std::views::transform([max_dist] (float val) { return max_dist > 0 ? (val / max_dist) : 0.0f;}) | std::ranges::to<std::vector<float>>();
        set_draw_mode = true;
    }

    void FieldEntity::build_texture() {
        glBindTexture(GL_TEXTURE_3D, tex3d);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, geometry_entity->model->nx, geometry_entity->model->ny,
            geometry_entity->model->nz, 0, GL_RED, GL_FLOAT, field.data()
        );
    
        glBindTexture(GL_TEXTURE_3D, tex3d_col);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F, geometry_entity->model->nx, geometry_entity->model->ny, 
            geometry_entity->model->nz, 0, GL_RGB, GL_FLOAT, color_field.data()); 
    }

    void FieldEntity::complete_set_traits() {
        if (worker_thread.joinable()) {
            worker_thread.join();
        }
        
        if (compute_passed) {
            build_texture();
        }
        dist_fld_lock.unlock();
    }

    void FieldEntity::set_traits(const std::vector<AxisDescMeta>& attrib_comps, const std::vector<Trait>& traits) {
        if (attrib_comps.empty()) {
            throw std::runtime_error("At least one attribute component required...");
        }

        if (traits.size() == 0) {
            throw std::runtime_error("set_traits() called with traits.size() == 0");
        }

        dist_fld_lock.lock();

        this->attrib_comps = attrib_comps;
        this->traits = traits;

        auto get_pt_norm = [this] (float val, size_t fld_dim) {
            auto pt_norm = val / (AXIS_LENGTH / 2);
            return 0.5 * ((this->attrib_comps[fld_dim].max_val + this->attrib_comps[fld_dim].min_val) + pt_norm * 
                (this->attrib_comps[fld_dim].max_val - this->attrib_comps[fld_dim].min_val));
        };

        for(auto& trait: this->traits) {
            switch(trait.type) {
                case TraitType::POINT: {
                    auto& tr_pt = std::get<Point>(trait.data);
                    auto x_f = get_pt_norm(tr_pt.x, 0);
                    if (attrib_comps.size() < 2) {
                        tr_pt.x = x_f;
                    } 
                    if (attrib_comps.size() >= 2) {
                        tr_pt.y = get_pt_norm(tr_pt.y, 1);
                    } 
                    break;
                }
                case TraitType::PARALLEL_POINT: {
                    auto& nd = std::get<NDPoint>(trait.data);
                    if (nd.ys.size() != this->attrib_comps.size()) nd.ys.resize(this->attrib_comps.size(), 0.0f);
                    for (size_t a = 0; a < this->attrib_comps.size(); ++a) {
                        nd.ys[a] = get_pt_norm(nd.ys[a], a);
                    }
                    break;
                }
                case TraitType::RANGE: {
                    auto& r = std::get<Range>(trait.data);
                    if (r.type == RangeType::INTERVAL) {
                        auto& tr_int = std::get<Interval>(r.range);
                        auto left = get_pt_norm(tr_int.left, 0);
                        auto right = get_pt_norm(tr_int.right, 0);
                        tr_int.left = std::min(left, right);
                        tr_int.right = std::max(left, right);
                    }
                    else if (r.type == RangeType::POLYGON) {
                        auto& tr_poly = std::get<Polygon>(r.range);
                        auto x_top = get_pt_norm(tr_poly.x_top, 0);
                        auto y_top = get_pt_norm(tr_poly.y_top, 1);
                        tr_poly.x_top = std::min(x_top, x_top + tr_poly.width);
                        tr_poly.y_top = std::min(y_top, y_top + tr_poly.height);
                        tr_poly.width = std::abs(tr_poly.width);
                        tr_poly.height = std::abs(tr_poly.height);
                    }
                    else if (r.type == RangeType::HYPERBOX) {
                        auto& hb = std::get<HyperBox>(r.range);
                        if (hb.yranges.size() != this->attrib_comps.size()) hb.yranges.resize(this->attrib_comps.size(), {0.0f, 0.0f});
                        for (size_t a = 0; a < this->attrib_comps.size(); ++a) {
                            float a0 = get_pt_norm(hb.yranges[a].first, a);
                            float a1 = get_pt_norm(hb.yranges[a].second, a);
                            hb.yranges[a].first = std::min(a0, a1);
                            hb.yranges[a].second = std::max(a0, a1);
                        }
                    }
                }
            }
        }

        worker_thread = std::thread([this] {
            stop_requested.store(false, std::memory_order_release);
            compute_passed = true;
            build_distance_field();
            advance_ui_clock(1, true);
        });
    }

    void FieldEntity::cancel_dist_computation() {
       stop_requested.store(true, std::memory_order_release);
       complete_set_traits();
        compute_passed = true;
    }
        
    void FieldEntity::set_isovalue(float value) {
        iso_value = value;
    }
        
    void FieldEntity::set_apply_color(bool apply_color) {
        is_apply_color = apply_color;   
    }

    void FieldEntity::clear_traits() {
        set_draw_mode = false;
    }

    void FieldEntity::draw() {
        if (!set_draw_mode) {
            return;
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, tex3d);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_3D, tex3d_col);
        
        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, res_x * res_y * res_z);
        glBindVertexArray(0); 
    }

    void FieldRenderer::init(int width, int height) {
        SpatialRenderer::init(width, height);
        entity.init(&this->SpatialRenderer::entity);
    }

    void FieldRenderer::render() {
        SpatialRenderer::render();

        if (!is_scene_setup) {
            return;
        }

        // There are 2 entities here. SpatialRenderer::entity takes care of transforms, camera, lighting, bounding box etc
        // Our entity is responsible for displaying computed distance field
        auto limits = Vector3f(SpatialRenderer::entity.model->nx, SpatialRenderer::entity.model->ny, SpatialRenderer::entity.model->nz);
		Matrix4f mvp = projection * camera.view * SpatialRenderer::entity.world * SpatialRenderer::entity.scale_transform * SpatialRenderer::entity.init_transform;
		Matrix4f mp = SpatialRenderer::entity.world * SpatialRenderer::entity.scale_transform * SpatialRenderer::entity.init_transform;
		auto light_position = light.get_position();
		auto camera_position = camera.get_position();
 
        auto pipeline = static_cast<IsoPipeline*>(pipelines[static_cast<int>(PipelineType::ISO)]);
        glUseProgram(pipeline->shader_program); 
		glUniformMatrix4fv(pipeline->uM, 1, GL_TRUE, &mp.m[0][0]);
		glUniformMatrix4fv(pipeline->uMVP, 1, GL_TRUE, &mvp.m[0][0]);
		glUniform3fv(pipeline->uLightPos, 1, light_position);
		glUniform3fv(pipeline->uViewPos, 1, camera_position);
        glUniform1f(pipeline->uIsoValue, entity.iso_value);
        glUniform3fv(pipeline->uOrigin, 1, SpatialRenderer::entity.model->origin);
        glUniform3fv(pipeline->uSpacing, 1, SpatialRenderer::entity.model->spacing);
        glUniform3fv(pipeline->uLimits, 1, limits);
        glUniform3fv(pipeline->uSteps, 1, entity.steps); 
        glUniform1i(pipeline->uApplyColor, entity.is_apply_color); 

        entity.draw();
    }

}