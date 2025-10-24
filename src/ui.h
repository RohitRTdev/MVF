#pragma once
#include <chrono>
#include <cmath>
#include <gtkmm.h>
#include <GL/glew.h>
#include "vtk.h"

class GLWidget : public Gtk::GLArea
{
public:
    GLWidget();
    ~GLWidget();

private:
    bool on_render(const Glib::RefPtr<Gdk::GLContext>& context);
    bool on_tick();
    void on_realize();

    sigc::connection _timeout_connection;
    std::chrono::steady_clock::time_point _t0 = std::chrono::steady_clock::now();
};

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

private:
    void on_file_open();
    void on_button1_clicked();
    bool file_load_handler();
    bool on_window_close();

    sigc::connection file_loader_conn;
    std::unique_ptr<LoadProxy> loader;
    std::string vtk_filename;

    Gtk::Box m_vbox{Gtk::Orientation::VERTICAL};
    Gtk::Box m_menubox{Gtk::Orientation::HORIZONTAL};
    Gtk::Box m_hbox{Gtk::Orientation::HORIZONTAL};
    Gtk::Box m_uibox{Gtk::Orientation::VERTICAL};
    
    Gtk::Label m_label;
    Gtk::Button m_button1{"Button 1"};
    GLWidget m_glarea;

    OverlayProgressBar progress_bar;
};
