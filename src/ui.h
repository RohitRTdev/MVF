#pragma once
#include <chrono>
#include <cmath>
#include <gtkmm.h>
#include <GL/glew.h>

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


class MainWindow : public Gtk::Window
{
public:
    MainWindow();
    virtual ~MainWindow() = default;

private:
    void on_button1_clicked();
    void on_button2_clicked();

    Gtk::Box m_hbox{Gtk::Orientation::HORIZONTAL};
    Gtk::Paned m_paned{Gtk::Orientation::HORIZONTAL};

    Gtk::Frame m_left_frame{""};
    Gtk::Box m_left{Gtk::Orientation::VERTICAL};
    Gtk::Label m_label;
    Gtk::Button m_button1{"Button 1"};
    Gtk::Button m_button2{"Button 2"};

    Gtk::Frame m_right_frame{""};
    GLWidget m_glarea;
};
