#pragma once
#include <gtkmm.h>
#include "renderer.h"

class MainWindow;
class SpatialPanel;
class AttributePanel;

namespace MVF {
    class RenderHandler : public Gtk::GLArea {
    public:
        RenderHandler(Renderer* renderer, bool is_spatial_handler = true);
   
        friend MainWindow;
        friend SpatialPanel;
        friend AttributePanel;
    private:
        Renderer* renderer;
        bool is_spatial_renderer;
        static bool initialized_glew;
        bool on_render(const Glib::RefPtr<Gdk::GLContext>& context);
        bool on_tick();
        void on_resize(int width, int height);
        void on_realize();
        void on_unrealize(); 
        bool on_key_pressed(guint keyval, guint keycode, Gdk::ModifierType state);
    };
}

