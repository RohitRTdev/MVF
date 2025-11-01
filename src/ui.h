#pragma once
#include <chrono>
#include <cmath>
#include <gtkmm.h>
#include <GL/glew.h>
#include "vtk.h"
#include "handler.h"
#include "renderer.h"

class OverlayProgressBar : public Gtk::Box {
public:
    OverlayProgressBar();
    void show();
    void hide();
    void set_fraction(double fraction);
private:
    Gtk::ProgressBar progress_bar;
    Gtk::Label label_progress;
};

class MainWindow : public Gtk::Window
{
public:
    MainWindow();
    virtual ~MainWindow() = default;

    friend MVF::RenderHandler;
private:
    void on_file_open();
    void on_button1_clicked();
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
    
    Gtk::Label m_label;
    Gtk::Button m_button1{"Button 1"};
    MVF::RenderHandler spatial_handler, attrib_handler;
    MVF::SpatialRenderer spatial_renderer;
    MVF::AttribRenderer attrib_renderer;

    OverlayProgressBar progress_bar;
};
