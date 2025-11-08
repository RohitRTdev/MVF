#pragma once
#include <gtkmm.h>

namespace MVF {
    class ErrorBox {
    public:
        ErrorBox() = default;
        ErrorBox(Gtk::Window* window, Gtk::Application* app);
        void error(const std::string& msg, bool exit = true); 
    private:
        Gtk::Window* window;
        Gtk::Application* app;
    };

    void app_error(const std::string& msg);
    void app_warn(const std::string& msg);
}
