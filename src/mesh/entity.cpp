#include <iostream>
#include <ranges>
#include <epoxy/gl.h>

#include "entity.h"
#include "math_utils.h"

namespace MVF {
    VolumeEntity::VolumeEntity() : Entity(Vector3f(0.0)) 
    {}

    void VolumeEntity::init(std::vector<Pipeline*>& pipelines) {
        this->pipelines = pipelines;
    }

    void VolumeEntity::resync() {
        create_vertex_array();
        create_bounding_box_buffers();
        create_buffers();

        if (type.mode == EntityMode::SCALAR_SLICE) {
            make_slice();
        }
        else if (type.mode == EntityMode::DVR) {
            update_dvr_resources();
        }
    }

    void VolumeEntity::load_model(std::shared_ptr<VolumeData>& data) {
        model = data;
        if (!arrow_buffer.is_active) {
            create_vertex_array();
        } 
        init_model_space();
        create_bounding_box_buffers();
        type.mode = EntityMode::NONE;
    }
        
    EntityMode VolumeEntity::get_mode() const {
        return type.mode;
    }
   
    void VolumeEntity::destroy_buffers(bool destroy_box) {
        if (destroy_box && box_buffer.is_active) {
            glDeleteVertexArrays(1, &box_buffer.vao_bound_box);
            glDeleteBuffers(2, std::array{box_buffer.vbo_box, box_buffer.ebo_box}.data());
            box_buffer.is_active = false;
        }

        if (vec_buffer.is_active) {
            glDeleteBuffers(1, &vec_buffer.vbo_glyph);
        }

        if (slice_buffer.is_active) {
            glDeleteTextures(1, &slice_buffer.tex3d);
            glDeleteBuffers(2, std::array{slice_buffer.vbo, slice_buffer.ebo}.data());
            glDeleteVertexArrays(1, &slice_buffer.vao);
            slice_buffer.is_active = false;
        }

        if (dvr_buffer.is_active) {
            glDeleteTextures(1, &dvr_buffer.tex3d);
            glDeleteBuffers(1, &dvr_buffer.vbo);
            glDeleteVertexArrays(1, &dvr_buffer.vao);
            dvr_buffer = {};
        }

        vec_buffer.is_active = false;

#ifdef MVF_DEBUG
        std::cout << "Deleted all model buffers..." << std::endl;
#endif
    }
   
    void VolumeEntity::set_box_mode() {
        type.mode = EntityMode::NONE;
        destroy_buffers(false);

        vec_buffer.is_active = false;
    }
   
    void VolumeEntity::set_vector_mode(const std::string& field1, const std::string& field2) {
        type.mode = EntityMode::VECTOR_GLYPH;
        type.data = VectorGlyphDesc{field1, field2};
        
        destroy_buffers(false);
        create_buffers(); 
    } 

    void VolumeEntity::set_vector_mode(const std::string& field1, const std::string& field2, const std::string& field3) { 
        type.mode = EntityMode::VECTOR_GLYPH;
        type.data = VectorGlyphDesc{field1, field2, field3};
        
        destroy_buffers(false);
        create_buffers();
    }

    void VolumeEntity::set_scalar_slice(const std::string& field, int axis) {
        type.mode = EntityMode::SCALAR_SLICE;
        type.data = ScalarSliceDesc{field, axis};
        
        destroy_buffers(false);
        create_buffers();
        make_slice();
    }

    void VolumeEntity::set_dvr(const std::string& field) {
        type.mode = EntityMode::DVR;
        type.data = DVRDesc{field};
        
        destroy_buffers(false);
        create_buffers();
        update_dvr_resources();
    }

    void VolumeEntity::set_slice_position(float t) {
        slice_t = std::min(1.0f, std::max(0.0f, t));
        if (type.mode == EntityMode::SCALAR_SLICE) {
            make_slice();
        }
    }

    void VolumeEntity::set_slice_axis(int axis) {
        auto& desc = std::get<ScalarSliceDesc>(type.data);
        axis = std::max(0, std::min(2, axis));
        if (desc.axis != axis) {
            desc.axis = axis;
            make_slice();
        }
    }

    void VolumeEntity::create_vertex_array() {
        // Generate mesh data for an arrow
        if (!initialized) {
            arrow_mesh = ArrowMesh(0.1f, 0.15f, 0.5f, 0.4f);
            initialized = true;
#ifdef MVF_DEBUG 
            std::cout << "Initialized static meshes..." << std::endl;
#endif
        }

        glGenVertexArrays(1, &arrow_buffer.vao_vec_glyph);
        glBindVertexArray(arrow_buffer.vao_vec_glyph);

        glGenBuffers(1, &arrow_buffer.vbo_arrow_mesh);
        glGenBuffers(1, &arrow_buffer.ebo_arrow_mesh);
        
        // Create the buffer for arrow mesh
        // 3 position + 3 normal
        glBindBuffer(GL_ARRAY_BUFFER, arrow_buffer.vbo_arrow_mesh);
        glBufferData(GL_ARRAY_BUFFER, arrow_mesh.vertices.size() * sizeof(ArrowVertex), arrow_mesh.vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ArrowVertex), 0);
        
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ArrowVertex), (void*)(3 * sizeof(float)));
        
        // Write the index/element buffer to GPU
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, arrow_buffer.ebo_arrow_mesh);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, arrow_mesh.indices.size() * sizeof(uint32_t), arrow_mesh.indices.data(), GL_STATIC_DRAW);
        
        // Unbind all the buffers
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        arrow_buffer.is_active = true;
#ifdef MVF_DEBUG
        std::cout << "Created static buffers..." << std::endl;
#endif
    }

    void VolumeEntity::create_bounding_box_buffers() {
        glGenVertexArrays(1, &box_buffer.vao_bound_box);
        glBindVertexArray(box_buffer.vao_bound_box);

        glGenBuffers(1, &box_buffer.vbo_box);
        glGenBuffers(1, &box_buffer.ebo_box);

        // Create the buffer for bounding box
        glBindBuffer(GL_ARRAY_BUFFER, box_buffer.vbo_box);
        glBufferData(GL_ARRAY_BUFFER, box.vertices.size() * sizeof(Vertex), box.vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
        
        // Write the index/element buffer to GPU
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, box_buffer.ebo_box);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, box.indices.size() * sizeof(uint32_t), box.indices.data(), GL_STATIC_DRAW);

        // Unbind all the buffers
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        box_buffer.is_active = true;

#ifdef MVF_DEBUG
        std::cout << "Created bounding box buffers..." << std::endl;
#endif
    }

    void VolumeEntity::create_buffers() {
        if (type.mode == EntityMode::VECTOR_GLYPH) {
            auto& desc = std::get<VectorGlyphDesc>(type.data); 
            field_mesh = GlyphMesh(model.get(), desc.field1, desc.field2, desc.field3);
            
            glBindVertexArray(arrow_buffer.vao_vec_glyph);
            glGenBuffers(1, &vec_buffer.vbo_glyph);
            glBindBuffer(GL_ARRAY_BUFFER, vec_buffer.vbo_glyph);
            glBufferData(GL_ARRAY_BUFFER, field_mesh.points.size() * sizeof(GlyphInstance), field_mesh.points.data(), GL_DYNAMIC_DRAW);
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GlyphInstance), 0);
            glVertexAttribDivisor(2, 1);
            
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(GlyphInstance), (void*)(3 * sizeof(float)));
            glVertexAttribDivisor(3, 1);
            
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(GlyphInstance), (void*)(6 * sizeof(float)));
            glVertexAttribDivisor(4, 1);
            
            // Unbind all the buffers
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            vec_buffer.is_active = true;
        }
        else if (type.mode == EntityMode::SCALAR_SLICE) {
            glGenTextures(1, &slice_buffer.tex3d);
            glBindTexture(GL_TEXTURE_3D, slice_buffer.tex3d);

            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 
          
            auto& field = model->scalars[std::get<ScalarSliceDesc>(type.data).field];
            auto [_, max_val] = std::minmax_element(field.begin(), field.end());
            std::vector<float> normalized = field | std::views::transform([max_val] (float val) {
                return val / *max_val;
            }) | std::ranges::to<std::vector>();

            glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, model->nx, model->ny, model->nz, 0, GL_RED, GL_FLOAT,   
                normalized.data()
            );            
            
            glGenVertexArrays(1, &slice_buffer.vao);

            glBindVertexArray(slice_buffer.vao);
            glGenBuffers(1, &slice_buffer.vbo);
            glGenBuffers(1, &slice_buffer.ebo);
            glBindBuffer(GL_ARRAY_BUFFER, slice_buffer.vbo);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTex), 0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTex), (void*)sizeof(Vertex));
            
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, slice_buffer.ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, slice_buffer.eb.size() * sizeof(uint32_t), slice_buffer.eb.data(), GL_STATIC_DRAW); 
            
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            slice_buffer.is_active = true;
        }
        else if(type.mode == EntityMode::DVR) {
            glGenVertexArrays(1, &dvr_buffer.vao);
            glBindVertexArray(dvr_buffer.vao);
            glGenBuffers(1, &dvr_buffer.vbo);
            glBindBuffer(GL_ARRAY_BUFFER, dvr_buffer.vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),0);
            glBindVertexArray(0);
            glGenTextures(1,&dvr_buffer.tex3d);

            dvr_buffer.is_active = true;
        }
#ifdef MVF_DEBUG
        std::cout << "Created model buffers..." << std::endl;
#endif
    } 
    
    void VolumeEntity::make_slice() {
        std::array<VertexTex, 4> vert;
        auto& desc = std::get<ScalarSliceDesc>(type.data);

        switch (desc.axis) {
            // X-axis
            case 0: {
                vert[0].x = box.vertices[0].x + slice_t * model->spacing.x * model->nx;   
                vert[0].y = box.vertices[0].y;   
                vert[0].z = box.vertices[0].z;
                vert[0].u = (vert[0].x - box.vertices[0].x) / (model->spacing.x * model->nx);
                vert[0].v = 0;
                vert[0].w = 0;            

                vert[1] = vert[0];
                vert[1].y = box.vertices[1].y;
                vert[1].v = 1;

                vert[2] = vert[1];
                vert[2].z = box.vertices[2].z;
                vert[2].w = 1;

                vert[3] = vert[2];
                vert[3].y = box.vertices[3].y;
                vert[3].v = 0;

                break;
            } 
            // Y-axis
            case 1: {
                vert[0].y = box.vertices[0].y + slice_t * model->spacing.y * model->ny;   
                vert[0].x = box.vertices[0].x;   
                vert[0].z = box.vertices[0].z;
                vert[0].v = (vert[0].y - box.vertices[0].y) / (model->spacing.y * model->ny);
                vert[0].u = 0;
                vert[0].w = 0;            

                vert[1] = vert[0];
                vert[1].z = box.vertices[2].z;
                vert[1].w = 1;

                vert[2] = vert[1];
                vert[2].x = box.vertices[4].x;
                vert[2].u = 1;

                vert[3] = vert[2];
                vert[3].z = box.vertices[4].z;
                vert[3].w = 0;

                break;
            } 
            // Z-axis
            case 2: {
                vert[0].z = box.vertices[0].z + slice_t * model->spacing.z * model->nz;   
                vert[0].x = box.vertices[0].x;   
                vert[0].y = box.vertices[0].y;
                vert[0].w = (vert[0].z - box.vertices[0].z) / (model->spacing.z * model->nz);
                vert[0].u = 0;
                vert[0].v = 0;            

                vert[1] = vert[0];
                vert[1].y = box.vertices[1].y;
                vert[1].v = 1;

                vert[2] = vert[1];
                vert[2].x = box.vertices[4].x;
                vert[2].u = 1;

                vert[3] = vert[2];
                vert[3].y = box.vertices[4].y;
                vert[3].v = 0;

                break;
            }
            default : {
                throw std::runtime_error("make_slice() called with axis > 2");
            } 
        }
        
        glBindVertexArray(slice_buffer.vao);
        glBindBuffer(GL_ARRAY_BUFFER, slice_buffer.vbo);

        glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(VertexTex), vert.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0); 
    }

    void VolumeEntity::update_dvr_resources() {
        auto& desc = std::get<DVRDesc>(type.data);
        auto it = model->scalars.find(desc.field); if (it==model->scalars.end()) return;
        auto& vec = it->second; 
        int nx=model->nx, ny=model->ny, nz=model->nz; dvr_buffer.nx=nx; dvr_buffer.ny=ny; dvr_buffer.nz=nz;
        std::vector<uint8_t> vol(nx*ny*nz);
        float minv=std::numeric_limits<float>::max(), maxv=-std::numeric_limits<float>::max();
        for(size_t i=0;i<vec.size(); ++i){ float v=vec[i]; minv=std::min(minv,v); maxv=std::max(maxv,v); }
        float denom=(maxv-minv)>0?(maxv-minv):1.f;
        for(int z=0; z<nz; ++z){ for(int y=0; y<ny; ++y){ for(int x=0; x<nx; ++x){ int idx=(z*ny + y)*nx + x; float sample; sample=vec[idx]; vol[idx]=(uint8_t)(255.f*(sample-minv)/denom); } } }
        glBindTexture(GL_TEXTURE_3D,dvr_buffer.tex3d);
        glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
        glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        glTexImage3D(GL_TEXTURE_3D,0,GL_R8,nx,ny,nz,0,GL_RED,GL_UNSIGNED_BYTE,vol.data());
    }

    void VolumeEntity::draw() {
		auto pipeline_box = reinterpret_cast<BoxPipeline*>(pipelines[static_cast<int>(PipelineType::BOX)]);
		auto pipeline_vec = reinterpret_cast<VecGlyphPipeline*>(pipelines[static_cast<int>(PipelineType::VEC_GLYPH)]);
		
        glUseProgram(pipeline_box->shader_program);
        glBindVertexArray(box_buffer.vao_bound_box);
        glDrawElements(GL_LINES, box.indices.size(), GL_UNSIGNED_INT, 0);

        if (type.mode == EntityMode::VECTOR_GLYPH) {
            glUseProgram(pipeline_vec->shader_program);
            glBindVertexArray(arrow_buffer.vao_vec_glyph);
            glDrawElementsInstanced(GL_TRIANGLES, arrow_mesh.indices.size(), GL_UNSIGNED_INT, 0, field_mesh.points.size());
            glBindVertexArray(0);
        }
        else if (type.mode == EntityMode::SCALAR_SLICE) {
            auto pipeline = reinterpret_cast<SlicePipeline*>(pipelines[static_cast<int>(PipelineType::SLICE)]);
            glUseProgram(pipeline->shader_program);
            glBindVertexArray(slice_buffer.vao);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_3D, slice_buffer.tex3d);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
        else if (type.mode == EntityMode::DVR) {
            auto pipeline = reinterpret_cast<DvrPipeline*>(pipelines[static_cast<int>(PipelineType::DVR)]);
            glUseProgram(pipeline->shader_program);
            glBindVertexArray(dvr_buffer.vao);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_3D, dvr_buffer.tex3d);
            glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, dvr_buffer.num_slices);
            glBindVertexArray(0);
        }
    }

    void VolumeEntity::compute_bounding_box() {
        float xmin = model->origin.x, ymin = model->origin.y, zmin = model->origin.z;
        float xmax = model->nx * model->spacing.x + model->origin.x; 
        float ymax = model->ny * model->spacing.y + model->origin.y; 
        float zmax = model->nz * model->spacing.z + model->origin.z; 

        box = BoundingBox(xmin, xmax, ymin, ymax, zmin, zmax);
    }

    void VolumeEntity::init_model_space() {
#ifdef MVF_DEBUG
        std::cout << "Initializing model space..." << std::endl;
#endif
        compute_bounding_box();

        position = model->origin;
        scale_transform.init_identity();
        world.init_identity();

        auto centre_x = model->origin.x + (model->spacing.x * model->nx) * 0.5;
        auto centre_y = model->origin.y + (model->spacing.y * model->ny) * 0.5;
        auto centre_z = model->origin.z + (model->spacing.z * model->nz) * 0.5;

        init_transform.init_translation_transform(-centre_x, -centre_y, -centre_z);
    }
    
    Entity::Entity(const Vector3f& position) {
        this->position = position;
        world.init_identity();
    }

    void Entity::rotate(float angle_x, float angle_y, float angle_z) {
        Matrix4f rotation;
        rotation.init_rotate_transform(angle_x, angle_y, angle_z);
        world = rotation * world;
    }

    void Entity::translate(float x, float y, float z) {
        Matrix4f translation;
        translation.init_translation_transform(x, y, z);
        world = translation * world;    
    }
        
    void VolumeEntity::scale(float factor) {
        scale_transform.init_scale_transform(factor, factor, factor);
    }
        
    void VolumeEntity::reset_transform() {
        scale_transform.init_identity();
        world.init_identity();
    }

    Vector3f Entity::get_position() {
        return world * position;
    }

    float* Entity::get_world_transform() {
        return &world.m[0][0];
    }

    CameraEntity::CameraEntity(const Vector3f& eye, const Vector3f& target, const Vector3f& up) : Entity(eye), 
    target(target), up(up) {
        view.init_camera_transform(position, target, up);
    }

    void CameraEntity::compute_view_matrix() {
        Vector3f position = get_position();
        
        view.init_camera_transform(position, target, up);
    }

    void CameraEntity::translate(float x, float y, float z) {
        Entity::translate(x, y, z);
        compute_view_matrix();
    }

    void CameraEntity::rotate(float angle_x, float angle_y, float angle_z) {
        Entity::rotate(angle_x, angle_y, angle_z);
        compute_view_matrix();
    } 

    LightEntity::LightEntity(const Vector3f& position, const Vector3f& color) : Entity(position) {
        this->color = color;
    }

}