#pragma once
#include <gtkmm.h>
#include <memory>
#include "renderer.h"
#include "widgets.h"

class MainWindow;
class SpatialPanel;
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

    private:
        bool on_key_pressed(guint keyval, guint keycode, Gdk::ModifierType state);
    };

    class AttribHandler : public RenderHandler {
    public:
        AttribHandler(AttribRenderer* renderer);
        void set_field_info(std::vector<AxisDesc>& descriptors);
        friend MainWindow;
        friend AttributePanel;

    private:
        HoverOverlay mouse_overlay;
        std::vector<std::string> field_comps;
        void on_mouse_click(int n_press, double x, double y);
    };
}



