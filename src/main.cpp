#include <iostream>
#include <locale>
#include <gtkmm.h>
#include "ui.h"

// Unstable hack to suppress the locale warning
extern "C" void suppress_locale_warning(const gchar *domain, GLogLevelFlags level, const gchar *message, gpointer user_data) {
    if (g_str_has_prefix(message, "Can't set the global") || g_str_has_prefix(message, "Can't make the global")) return;
    g_log_default_handler(domain, level, message, user_data);
}

int main(int argc, char* argv[])
{
    g_log_set_default_handler(suppress_locale_warning, nullptr);

    auto app = Gtk::Application::create("mvf.app");
    
    auto css_provider = Gtk::CssProvider::create();
    css_provider->load_from_path("assets/main.css");
    Gtk::StyleContext::add_provider_for_display(
        Gdk::Display::get_default(),
        css_provider,
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    return app->make_window_and_run<MainWindow>(argc, argv);
}
