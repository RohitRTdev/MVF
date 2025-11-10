#pragma once

#include <string>
#include <vector>
#include <GL/glew.h>

namespace MVF {
    enum class PipelineType {
        // Spatial domain
        VEC_GLYPH = 0,
        BOX,
        SLICE,
        DVR,

        // Attribute domain
        AXIS = 0,
        MARKER
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
        GLuint uMVP, uColor;
        
        BoxPipeline();    
    };

    struct SlicePipeline : Pipeline {
        GLuint uMVP;
        GLuint uTex;
        SlicePipeline();
    };

    struct DvrPipeline : Pipeline {
        GLuint uMVP;
        GLuint uTex3D;
        GLuint uBBoxMin;
        GLuint uBBoxMax;
        GLuint uSlices;
        GLuint uAlphaScale;
        DvrPipeline();
    };
    
    struct AxisPipeline : Pipeline {
        GLuint uColor;
        AxisPipeline();    
    };
    
    struct MarkerPipeline: Pipeline {
        GLuint uColor;
        MarkerPipeline();    
    };
}
