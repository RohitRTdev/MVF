#pragma once

#include <string>
#include <GL/glew.h>

namespace MVF {
    enum class PipelineType {
        VEC_GLYPH,
        BOX
    };

    struct Pipeline {
        GLuint shader_program;
        PipelineType type;

        Pipeline(PipelineType type); 
        Pipeline(const std::string& vs_file, const std::string& fs_file, PipelineType type);
        Pipeline(const std::string& vs_file, const std::string& gs_file, const std::string& fs_file, PipelineType type);

        static void init_pipelines();

    protected:
        GLuint get_uniform_var(const std::string& var_name); 
    
    private:
        void add_shader(const char* shader_text, GLenum shader_type); 
        void compile_shaders(const std::string& vs_file, const std::string& gs_file, const std::string& fs_file); 
    };

    struct VecGlyphPipeline : Pipeline {
        GLuint uMVP, uM;
        GLuint uLightPos, uViewPos;
        
        VecGlyphPipeline();    
    };
    
    struct BoxPipeline : Pipeline {
        GLuint uMVP;
        
        BoxPipeline();    
    };
}

extern std::vector<MVF::Pipeline*> pipelines;
