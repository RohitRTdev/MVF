#pragma once
#include <chrono>
#include <cmath>
#include <functional>
#include <gtkmm.h>
#include <epoxy/gl.h>
#include "vtk.h"
#include "handler.h"
#include "renderer.h"
#include "panel.h"
#include "widgets.h"

class MainWindow : public Gtk::Window
{
public:
    MainWindow();
    virtual ~MainWindow() = default;
    

    friend MVF::RenderHandler;
private:

    struct ButtonDescriptor {
        std::string tooltip_text;
        std::string icon_filename;
        std::function<void ()> handler;
    };

    void on_file_open();
    bool file_load_handler();
    bool on_window_close();
    bool on_key_press();
    void toggle_attrib_space();
    Gtk::Box* build_menu(const std::vector<ButtonDescriptor>& desc);

    bool is_toggle_on = false;
    bool is_attrib_space_visible = false;

    sigc::connection file_loader_conn;
    std::unique_ptr<MVF::LoadProxy> loader;
    std::string vtk_filename;

    Gtk::Box m_vbox{Gtk::Orientation::VERTICAL};
    Gtk::Box m_menubox{Gtk::Orientation::HORIZONTAL};
    Gtk::Box m_hbox{Gtk::Orientation::HORIZONTAL};
    Gtk::Box m_uibox{Gtk::Orientation::VERTICAL};
    Gtk::Paned pane;

    Gtk::Box spatial_box{Gtk::Orientation::VERTICAL}, attrib_box{Gtk::Orientation::VERTICAL}, field_box{Gtk::Orientation::VERTICAL};
    
    MVF::SpatialHandler spatial_handler, field_handler;
    MVF::AttribHandler attrib_handler;
    MVF::SpatialRenderer spatial_renderer;
    MVF::AttribRenderer attrib_renderer;
    MVF::FieldRenderer field_renderer;
    SpatialPanel spatial_panel;
    AttributePanel attrib_panel; 

    OverlayProgressBar progress_bar;
};
