#include <iostream>
#include <epoxy/gl.h>

#include "entity.h"
#include "math_utils.h"

namespace MVF {
    VolumeEntity::VolumeEntity() : Entity(Vector3f(0.0)) 
    {}

    void VolumeEntity::init(std::vector<Pipeline*>& pipelines) {
        this->pipelines = pipelines;
        create_vertex_array();
    }

    void VolumeEntity::resync() {
        create_vertex_array();
        create_bounding_box_buffers();
        create_buffers();
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
   
    void VolumeEntity::destroy_buffers() {
        if (box_buffer.is_active) {
            glDeleteVertexArrays(1, &box_buffer.vao_bound_box);
            glDeleteBuffers(2, std::array{box_buffer.vbo_box, box_buffer.ebo_box}.data());
        }

        if (vec_buffer.is_active) {
            glDeleteBuffers(1, &vec_buffer.vbo_glyph);
        }

        vec_buffer.is_active = false;
        box_buffer.is_active = false;

#ifdef MVF_DEBUG
        std::cout << "Deleted all model buffers..." << std::endl;
#endif
    }
   
    void VolumeEntity::set_box_mode() {
        type.mode = EntityMode::NONE;
        if (vec_buffer.is_active) {
            glDeleteBuffers(1, &vec_buffer.vbo_glyph);
        }

        vec_buffer.is_active = false;
    }
    
    void VolumeEntity::set_vector_mode(const std::string& field1, const std::string& field2, const std::string& field3) { 
        type.mode = EntityMode::VECTOR_GLYPH;
        type.data = VectorGlyphDesc{field1, field2, field3};

        create_buffers();
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

    #ifdef MVF_DEBUG
            std::cout << "Created model buffers..." << std::endl;
    #endif
        }
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