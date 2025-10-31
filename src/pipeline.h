#pragma once

#include <string>
#include <GL/glew.h>

namespace MVF {
    enum class PipelineType {
        VEC_GLYPH = 0,
        BOX,
        AXIS = 0
    };

    struct Pipeline {
        GLuint shader_program;
        PipelineType type;

        Pipeline(PipelineType type); 
        Pipeline(const std::string& vs_file, const std::string& fs_file, PipelineType type);
        Pipeline(const std::string& vs_file, const std::string& gs_file, const std::string& fs_file, PipelineType type);

        static std::vector<Pipeline*> init_pipelines(bool is_spatial_pipeline = true);

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
    
    struct AxisPipeline : Pipeline {
        AxisPipeline();    
    };
}
