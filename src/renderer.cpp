#include <string> 
#include <GL/glew.h>

#include "vtk.h"
#include "entity.h"
#include "renderer.h"
#include "math_utils.h"
#include "pipeline.h"

constexpr float VIEW_THRESHOLD = 3.0f;

namespace MVF {
	Renderer::Renderer() : camera(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f)),
	light(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(1.0f, 1.0f, 1.0f)) {

	}

	void Renderer::init(int width, int height) {
		glEnable(GL_DEPTH_TEST); 
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		set_viewport(width, height);

#ifdef MVF_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback([](GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar* msg, const void*) {
			std::cerr << "GL DEBUG: " << msg << std::endl;
		}, nullptr);
#endif
	}
        
	void SpatialRenderer::resync() {
		if (!is_scene_setup) {
			return;
		}	

		entity.resync();
	}
	
	void SpatialRenderer::unload() {
		entity.destroy_buffers();
	}

	void SpatialRenderer::init(int width, int height) {
		Renderer::init(width, height);
		glClearColor(0.25f, 0.25f, 0.27f, 1.0f);
		pipelines = Pipeline::init_pipelines();
		entity.init(pipelines);	
	}

	void SpatialRenderer::setup_scene(std::shared_ptr<VolumeData>& data) {
		purge_scene();
		entity.load_model(data);
		
		// Get the minimum clipping box that can enclose the mesh
		auto v0 = entity.box.vertices[0];

		// Get the box in world coordinates
		auto vw0 = entity.init_transform * v0;

		// Place the camera such that the entire mesh is visible
		// Camera is at some (0, 0, z) and is looking at the origin
		// Here, the near and far planes should be set with respect to camera position
		// This is because we apply the projection after applying the camera transformation, so 
		// we're now in camera space
		auto x_bound = std::abs(vw0.x) + VIEW_THRESHOLD;
		auto y_bound = std::abs(vw0.y) + VIEW_THRESHOLD;
		auto z_bound = std::abs(vw0.z) + VIEW_THRESHOLD;
		
		auto zNear = 5.0f;
		auto zFar = 2000.0f;
		
		camera.world.init_identity();
		camera.translate(x_bound, y_bound, z_bound * 5.0f);

		OrthoProjInfo info {.bottom = -y_bound, .top = y_bound, 
		.left = -x_bound, .right = x_bound, .zNear = zNear, .zFar = zFar};
		
		projection.init_ortho_proj_transform(info);
		
		light.world.init_identity();
		light.translate(x_bound + 10, y_bound + 10, z_bound + 10);
		
		is_scene_setup = true;	
	}
	
	void SpatialRenderer::purge_scene() {
		if (!is_scene_setup) {
			return;
		}

		entity.destroy_buffers(false);
		is_scene_setup = false;
	} 

	void Renderer::set_viewport(int width, int height) {
		glViewport(0, 0, width, height);
		this->width = width;
		this->height = height;
	}

	void SpatialRenderer::render() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if(!is_scene_setup) {
			return;
		}

		Matrix4f mvp = projection * camera.view * entity.world * entity.scale_transform * entity.init_transform;
		auto light_position = light.get_position();
		auto camera_position = camera.get_position();

		auto pipeline_box = reinterpret_cast<BoxPipeline*>(pipelines[static_cast<int>(PipelineType::BOX)]);
		glUseProgram(pipeline_box->shader_program);
		glUniformMatrix4fv(pipeline_box->uMVP, 1, GL_TRUE, &mvp.m[0][0]);

		auto pipeline_vec = reinterpret_cast<VecGlyphPipeline*>(pipelines[static_cast<int>(PipelineType::VEC_GLYPH)]);
		glUseProgram(pipeline_vec->shader_program);
		
		glUniformMatrix4fv(pipeline_vec->uM, 1, GL_TRUE, &entity.world.m[0][0]);
		glUniformMatrix4fv(pipeline_vec->uMVP, 1, GL_TRUE, &mvp.m[0][0]);
		glUniform3fv(pipeline_vec->uLightPos, 1, light_position);
		glUniform3fv(pipeline_vec->uViewPos, 1, camera_position);
		
		entity.draw();
	}
}
