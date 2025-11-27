#pragma once

#include <epoxy/gl.h>
#include <vector>
#include <string>

#include "entity.h"
#include "math_utils.h"
#include "attrib.h"

struct _cairo; // forward declare cairo_t
typedef struct _cairo cairo_t;

namespace MVF {
    class Renderer {
    public:
        Renderer();
        virtual void render() = 0;
        virtual void resync() = 0;
        virtual void init(int width, int height);
        void set_viewport(int width, int height);
    protected:
        CameraEntity camera;
        LightEntity light;
        Matrix4f projection;
        std::vector<Pipeline*> pipelines;
    private:
        int width, height;
    };

    class SpatialRenderer : public Renderer {
    public:
        VolumeEntity entity;
        void init(int width, int height) override;
        void setup_scene(std::shared_ptr<VolumeData>& data);
        void purge_scene();
        void render() override;
        void resync() override;
        void set_world_orientation(const Matrix4f& R);
    protected:
        bool is_scene_setup = false;
    };

    class FieldRenderer : public SpatialRenderer {
    public:
        FieldEntity entity;
        void init(int width, int height);
        void render() override;
    };

    class AttribRenderer : public Renderer {
    public:
        void init(int width, int height) override;
        void render() override;
        void resync() override;
        void set_field_data(std::shared_ptr<VolumeData>& vol);
        void set_attrib_space_axis(const std::vector<AxisDesc>& descriptors);
        float get_field_point(float t, size_t id);
        std::pair<std::vector<AxisDescMeta>, std::vector<Trait>> get_traits();
        void set_point_trait(float x);
        void set_point_trait(float x, float y);
        void set_range_trait(float x1, float x2);
        void set_range_trait(float x_top, float y_top, float width, float height);
        void modify_range_trait(float x_right);
        void modify_range_trait(float x_end, float y_end);
        void enable_plot(bool enable);
        void clear_traits();
        void set_sample_period(size_t period);
        void cycle_sample_period();
        std::pair<float, float> screen_to_world(double sx, double sy);
        std::pair<float, float> world_to_screen(float wx, float wy);
        void on_mouse_move(double mx, double my);
        void draw_overlay_cairo(cairo_t* cr);
        // update viewport size for overlay math
        void update_viewport_size(int w, int h) { viewport_width = (float)w; viewport_height = (float)h; }
        // parallel selection APIs
        void select_parallel_line_by_points(const std::vector<float>& axis_world_ys);
        void select_parallel_region_by_ranges(const std::vector<std::pair<float,float>>& axis_world_ranges);
        // pending selection markers (crosses) for parallel coordinates
        void clear_parallel_pending_markers();
        void add_parallel_pending_marker(size_t axis_index, float world_y);
    private:
        Axis axis_mesh_x, axis_mesh_y;
        PointMarker marker;
        GLuint vao_x_axis=0, vao_y_axis=0, vao_marker=0, vao_interval=0;
        GLuint vao_polyline=0, vao_polypoint=0, vao_scatterplot=0;
        GLuint vao_distplotsolid=0, vao_distplotlines=0;
        // Parallel coordinates VAOs
        GLuint vao_parallel_axes=0, vao_parallel_lines=0;
        // highlighted/region VAOs
        GLuint vao_parallel_lines_highlight=0, vao_parallel_lines_region=0;
        // VAO for filled region triangles
        GLuint vao_parallel_region_fill=0;
        // VAO for pending parallel markers
        GLuint vao_parallel_marker=0;
        GLuint vbo_x_axis=0, vbo_y_axis=0, vbo_marker=0, vbo_marker_pos=0, vbo_interval=0;
        GLuint vbo_polyline=0, vbo_polypoint=0, vbo_scatterplot=0;
        GLuint vbo_distplotsolid=0, vbo_distplotlines=0;
        // Parallel coordinates VBOs
        GLuint vbo_parallel_axes=0, vbo_parallel_lines=0;
        // highlighted/region VBOs
        GLuint vbo_parallel_lines_highlight=0, vbo_parallel_lines_region=0;
        // VBO for filled region triangles
        GLuint vbo_parallel_region_fill=0;
        // VBO for pending parallel markers
        GLuint vbo_parallel_marker_pos=0;
        size_t num_interval_vertices = 0, num_range_tri_vertices = 0, num_range_pt_vertices = 0;
        std::shared_ptr<VolumeData> data;
        std::vector<AxisDescMeta> descriptors;
        std::vector<Trait> traits;
        std::vector<Vector2f> scatter_plot, dist_plot_solid, dist_plot_lines;
        // Parallel coordinates storage
        std::vector<Vector2f> parallel_axes_vertices;
        std::vector<Vector2f> parallel_lines_vertices;
        // selections storage
        std::vector<Vector2f> parallel_highlight_lines_vertices;
        std::vector<Vector2f> parallel_region_lines_vertices; // outline
        std::vector<Vector2f> parallel_region_fill_vertices;  // triangles
        // pending markers storage
        std::vector<Vector2f> parallel_pending_marker_positions;
        bool has_pending_markers = false;
        bool is_plot_visible = false;
        size_t sample_period = 1000; // default 1-in-1000 sampling for parallel coordinates
        // overlay state
        float viewport_width = 0.0f, viewport_height = 0.0f;
        int hovered_axis = -1;
        float hovered_world_y = 0.0f;
        std::string hover_text;
        void setup_buffers();
        size_t last_chosen_id = 0;

        void setup_buffers(); 
        void setup_traits();
        void generate_scatter_plot();
        void generate_freq_distribution();
        void generate_parallel_coordinates();
        void setup_plot();
        void clear_plot();
        size_t get_color_id();
    };
}
