#include <iostream>
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
            update_slice_resources();
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
        }

        if (vec_buffer.is_active) {
            glDeleteBuffers(1, &vec_buffer.vbo_glyph);
        }

        if (slice_buffer.is_active) {
            glDeleteTextures(1, &slice_buffer.tex_slice);
            glDeleteBuffers(1, &slice_buffer.vbo_slice);
            glDeleteVertexArrays(1, &slice_buffer.vao_slice);
            slice_buffer = {};
        }

        if (dvr_buffer.is_active) {
            glDeleteTextures(1, &dvr_buffer.tex3d);
            glDeleteBuffers(1, &dvr_buffer.vbo);
            glDeleteVertexArrays(1, &dvr_buffer.vao);
            dvr_buffer = {};
        }

        vec_buffer.is_active = false;
        box_buffer.is_active = false;

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
        update_slice_resources();
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
            update_slice_resources();
        }
    }

    // New helpers
    void VolumeEntity::set_slice_axis(int axis) {
        if (type.mode != EntityMode::SCALAR_SLICE) return;
        auto& desc = std::get<ScalarSliceDesc>(type.data);
        axis = std::max(0, std::min(2, axis));
        if (desc.axis != axis) {
            desc.axis = axis;
            update_slice_resources();
        }
    }

    int VolumeEntity::get_slice_axis() const {
        if (type.mode != EntityMode::SCALAR_SLICE) return -1;
        const auto& desc = std::get<ScalarSliceDesc>(type.data);
        return desc.axis;
    }

    float VolumeEntity::get_slice_position() const {
        return slice_t;
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
            glGenVertexArrays(1, &slice_buffer.vao_slice);
            glBindVertexArray(slice_buffer.vao_slice);
            glGenBuffers(1, &slice_buffer.vbo_slice);
            glBindBuffer(GL_ARRAY_BUFFER, slice_buffer.vbo_slice);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
            glBindVertexArray(0);
            glGenTextures(1, &slice_buffer.tex_slice);
            
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

    // Revised: pass full ranges to generate correct slice plane geometry
    static void make_slice_quad(float xmin, float xmax,
                                float ymin, float ymax,
                                float zmin, float zmax,
                                float p, int axis,
                                std::array<float, 20>& out) {
        if (axis == 2) { // Z constant, vary X,Y
            float v[20] = {
                xmin, ymin, p, 0.0f, 0.0f,
                xmax, ymin, p, 1.0f, 0.0f,
                xmin, ymax, p, 0.0f, 1.0f,
                xmax, ymax, p, 1.0f, 1.0f
            };
            std::copy(v, v+20, out.begin());
        } else if (axis == 1) { // Y constant, vary X,Z
            float v[20] = {
                xmin, p, zmin, 0.0f, 0.0f,
                xmax, p, zmin, 1.0f, 0.0f,
                xmin, p, zmax, 0.0f, 1.0f,
                xmax, p, zmax, 1.0f, 1.0f
            };
            std::copy(v, v+20, out.begin());
        } else { // axis == 0, X constant, vary Y,Z
            float v[20] = {
                p, ymin, zmin, 0.0f, 0.0f,
                p, ymin, zmax, 1.0f, 0.0f,
                p, ymax, zmin, 0.0f, 1.0f,
                p, ymax, zmax, 1.0f, 1.0f
            };
            std::copy(v, v+20, out.begin());
        }
    }

    void VolumeEntity::update_slice_resources() {
        auto& desc = std::get<ScalarSliceDesc>(type.data);
        
        // Update plane vertices based on slice_t
        float xmin = model->origin.x, ymin = model->origin.y, zmin = model->origin.z;
        float xmax = model->nx * model->spacing.x + model->origin.x; 
        float ymax = model->ny * model->spacing.y + model->origin.y; 
        float zmax = model->nz * model->spacing.z + model->origin.z; 
        float p = 0;
        if (desc.axis == 2) {
            p = zmin + (zmax - zmin) * slice_t; // Z const
        } else if (desc.axis == 1) {
            p = ymin + (ymax - ymin) * slice_t; // Y const
        } else {
            p = xmin + (xmax - xmin) * slice_t; // X const
        }
        std::array<float, 20> verts;
        make_slice_quad(xmin, xmax, ymin, ymax, zmin, zmax, p, desc.axis, verts);
        glBindBuffer(GL_ARRAY_BUFFER, slice_buffer.vbo_slice);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*verts.size(), verts.data(), GL_DYNAMIC_DRAW);

        // Upload scalar values as a R8 texture normalized 0..1
        auto it = model->scalars.find(desc.field);
        if (it == model->scalars.end()) return;
        auto& vec = it->second;
        // Determine slice dimensions
        int w=0,h=0; int nx=model->nx, ny=model->ny, nz=model->nz;
        if (desc.axis == 2) { w = nx; h = ny; }
        else if (desc.axis == 1) { w = nx; h = nz; }
        else { w = ny; h = nz; }
        slice_buffer.tex_w = w; slice_buffer.tex_h = h;
        std::vector<uint8_t> tex(w*h);
        // Compute min/max of magnitude (or scalar) across whole volume
        float minv = std::numeric_limits<float>::max(), maxv = -std::numeric_limits<float>::max();
        for (size_t i=0;i<vec.size(); i+=1) {
            float v = vec[i]; minv = std::min(minv, v); maxv = std::max(maxv, v);
        }
        float denom = (maxv-minv) > 0 ? (maxv-minv) : 1.0f;
        int k = int(slice_t * ((desc.axis==2? nz: desc.axis==1? ny: nx)-1));
        for (int j=0;j<h;++j) {
            for (int i=0;i<w;++i) {
                int idx;
                if (desc.axis == 2) idx = (k*ny + j)*nx + i; // z slice
                else if (desc.axis == 1) idx = (j*ny + k)*nx + i; // y slice
                else idx = (j*ny + i)*nx + k; // x slice
                float sample;
                sample = vec[idx];
                tex[j*w + i] = (uint8_t)(255.0f * (sample - minv) / denom);
            }
        }
        glBindTexture(GL_TEXTURE_2D, slice_buffer.tex_slice);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, tex.data());
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
            glBindVertexArray(slice_buffer.vao_slice);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, slice_buffer.tex_slice);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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