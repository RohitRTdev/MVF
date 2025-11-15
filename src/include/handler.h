#pragma once

#include <gtkmm.h>
#include <memory>
#include "renderer.h"
#include "widgets.h"
#include "math_utils.h"

class MainWindow;
class SpatialPanel;
class FieldPanel;
class AttributePanel;

namespace MVF {
    class RenderHandler : public Gtk::GLArea {
    public:
        RenderHandler(Renderer* renderer);
        
    protected:
        Renderer* renderer;

    private:
        static bool initialized_glew;
        bool on_render(const Glib::RefPtr<Gdk::GLContext>& context);
        void on_resize(int width, int height);
        void on_realize();
        void on_unrealize(); 
    };

    class SpatialHandler : public RenderHandler {
    public:
        SpatialHandler(SpatialRenderer* renderer);
        void reset_camera();
        friend MainWindow;
        friend SpatialPanel;
        friend FieldPanel;

    private:
        bool on_key_pressed(guint keyval, guint keycode, Gdk::ModifierType state);
        // Trackball state
        float current_zoom = 1.0f;
        bool trackball_active = false;
        int last_x = 0, last_y = 0;
        Quaternion trackball_quat = Quaternion(1,0,0,0);
    };

    class AttribHandler : public RenderHandler {
    public:
        AttribHandler(AttribRenderer* renderer);
        void set_field_info(std::vector<AxisDesc>& descriptors);
        void clear_field_info();
        void set_point_trait_mode(bool set_point_trait);
        friend MainWindow;
        friend AttributePanel;

    private:
        bool handle_traits = false;
        bool is_point_trait = true;
        bool activate_range_selection = false; 
        HoverOverlay mouse_overlay;
        std::vector<std::string> field_comps;
        void on_mouse_click(int n_press, double x, double y);
    };
}



