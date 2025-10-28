#pragma once
#include <gtkmm.h>

class MainWindow;

namespace MVF {
    class RenderHandler : public Gtk::GLArea {
    public:
        RenderHandler(MainWindow* parent);
   
        friend MainWindow;
    private:
        MainWindow* parent;
        bool on_render(const Glib::RefPtr<Gdk::GLContext>& context);
        bool on_tick();
        void on_resize(int width, int height);
        void on_realize();
        bool on_key_pressed(guint keyval, guint keycode, Gdk::ModifierType state);
    };
}

