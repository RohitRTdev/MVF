#pragma once
#include <chrono>
#include <cmath>
#include <gtkmm.h>
#include <GL/glew.h>
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
    void on_file_open();
    bool file_load_handler();
    bool on_window_close();
    bool on_key_press();

    sigc::connection file_loader_conn;
    std::unique_ptr<MVF::LoadProxy> loader;
    std::string vtk_filename;

    Gtk::Box m_vbox{Gtk::Orientation::VERTICAL};
    Gtk::Box m_menubox{Gtk::Orientation::HORIZONTAL};
    Gtk::Box m_hbox{Gtk::Orientation::HORIZONTAL};
    Gtk::Box m_uibox{Gtk::Orientation::VERTICAL};
    
    MVF::SpatialHandler spatial_handler;
    MVF::AttribHandler attrib_handler;
    MVF::SpatialRenderer spatial_renderer;
    MVF::AttribRenderer attrib_renderer;
    SpatialPanel spatial_panel;
    AttributePanel attrib_panel; 

    OverlayProgressBar progress_bar;
};
