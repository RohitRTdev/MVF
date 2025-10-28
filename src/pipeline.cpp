#include "vtk.h"
#include "error.h"
#include "pipeline.h"

std::vector<MVF::Pipeline*> pipelines;

namespace MVF{
    Pipeline::Pipeline(PipelineType type) : type(type) {
        shader_program = glCreateProgram();
        if (shader_program == 0) {
            std::cerr << "Error creating shader program" << std::endl;
            exit(1);
        }
    }
    
    Pipeline::Pipeline(const std::string& vs_file, const std::string& fs_file, PipelineType type) : Pipeline(type) {
        compile_shaders(vs_file, "", fs_file);
    }
	
    Pipeline::Pipeline(const std::string& vs_file, const std::string& gs_file, const std::string& fs_file, PipelineType type) : Pipeline(type) {
        compile_shaders(vs_file, gs_file, fs_file);
    }
    
    void Pipeline::add_shader(const char* shader_text, GLenum shader_type) {
		GLuint shader_obj = glCreateShader(shader_type);

		if (shader_obj == 0) {
			std::cerr << "Error creating shader type: " << shader_type << std::endl;
            exit(1);
		}

		const GLchar* p = shader_text;
		glShaderSource(shader_obj, 1, &p, nullptr);
		glCompileShader(shader_obj);
		
		GLint success;
		glGetShaderiv(shader_obj, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLchar info_log[1024];
			glGetShaderInfoLog(shader_obj, 1024, NULL, info_log);
            std::cerr << "Error compiling shader type " << shader_type <<": " << info_log << std::endl;
			exit(1);
		}

		glAttachShader(shader_program, shader_obj);
	}
 
    void Pipeline::compile_shaders(const std::string& vs_file, const std::string& gs_file, const std::string& fs_file) {
		std::string vs, gs, fs;
        
        if (!read_file(vs_file, vs)) {
			std::cerr << "Vertex shader file missing! -> " << vs_file << std::endl;
            exit(1);
		}

        if(!gs_file.empty()) {
            if (!read_file(gs_file, gs)) {
                std::cerr << "Geometry shader file missing! -> " << gs_file << std::endl;
                exit(1);
            }
        }

		if (!read_file(fs_file, fs)) {
            std::cerr << "Fragment shader file missing! -> " << fs_file << std::endl;
            exit(1);
		}

		add_shader(vs.c_str(), GL_VERTEX_SHADER);
        if(!gs_file.empty()) {
            add_shader(gs.c_str(), GL_GEOMETRY_SHADER);
        }
		add_shader(fs.c_str(), GL_FRAGMENT_SHADER);

		GLint success = 0;
		GLchar error_log[1024] = {0};
		
		glLinkProgram(shader_program);
		glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
		if (success == 0) {
			glGetProgramInfoLog(shader_program, sizeof(error_log), NULL, error_log);
            std::cerr << "Error linking shader program: " << error_log << std::endl;
			exit(1);
		}

		glUseProgram(shader_program);
		
		glValidateProgram(shader_program);
		glGetProgramiv(shader_program, GL_VALIDATE_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shader_program, sizeof(error_log), NULL, error_log);
            std::cerr << "Invalid shader program: " << error_log << std::endl;
			exit(1);
		}
	}

    GLuint Pipeline::get_uniform_var(const std::string& var_name) {
		GLuint u_var = glGetUniformLocation(shader_program, var_name.c_str());
		
		GLenum error_code = glGetError();
		if (error_code != GL_NO_ERROR) {
			std::cerr << "Uniform error: " << error_code << std::endl;
            exit(1);
		}

        return u_var;
    }
    
    VecGlyphPipeline::VecGlyphPipeline() : Pipeline("shaders/vec_glyph.vs", "shaders/vec_glyph.fs", PipelineType::VEC_GLYPH) {
        uMVP = get_uniform_var("uMVP");
        uM = get_uniform_var("uM");
        uLightPos = get_uniform_var("uLightPos");
        uViewPos = get_uniform_var("uViewPos");
    }
        
    BoxPipeline::BoxPipeline() : Pipeline("shaders/box.vs", "shaders/box.fs", PipelineType::BOX) {
        uMVP = get_uniform_var("uMVP");
    }  

    void Pipeline::init_pipelines() {
        // *This must follow the same order as of the PipelineType enum class*
        pipelines.assign({new VecGlyphPipeline(), new BoxPipeline()});
    
#ifdef MVF_DEBUG
        std::cout << "Recreated " << pipelines.size() << " pipelines" << std::endl;
#endif
    }
}