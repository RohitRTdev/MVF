#include "error.h"

MVF::ErrorBox main_error_box;

namespace MVF {
    ErrorBox::ErrorBox(Gtk::Window* window, Gtk::Application* app) : window(window), app(app) 
    {}

    void ErrorBox::error(const std::string& msg, bool exit) {
        auto dialog = Gtk::make_managed<Gtk::MessageDialog>(*window, msg, false, Gtk::MessageType::ERROR, 
            Gtk::ButtonsType::OK, true);

        dialog->set_title("Error");
        dialog->set_transient_for(*window);
        dialog->set_modal(true);

        // Run the dialog modally
        dialog->set_visible(true);

        dialog->signal_response().connect([this, exit, dialog] (int) {
            dialog->destroy();
            if (exit) {
                window->close();
                app->quit();  
            }
        }); 

        dialog->show();
        window->get_display()->beep();
    }

    void app_error(const std::string& msg) {
        main_error_box.error(msg);
    }

    void app_warn(const std::string& msg) {
        main_error_box.error(msg, false);
    }
}

