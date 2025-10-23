#include "ui.h"
#include <iostream>

GLWidget::GLWidget()
{
    set_hexpand(true);
    set_vexpand(true);

    signal_realize().connect(sigc::mem_fun(*this, &GLWidget::on_realize));
    signal_render().connect(sigc::mem_fun(*this, &GLWidget::on_render), true);

    _timeout_connection = Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &GLWidget::on_tick), 16);
}

GLWidget::~GLWidget()
{
    _timeout_connection.disconnect();
}

void GLWidget::on_realize()
{
    Gtk::GLArea::on_realize();

    try {
        make_current();
    } catch (const Gdk::GLError& gle) {
        std::cerr << "Failed to realize GLArea: " << gle.what() << std::endl;
    }

    glEnable(GL_DEPTH_TEST);
}

bool GLWidget::on_render(const Glib::RefPtr<Gdk::GLContext>& context)
{
    using clk = std::chrono::steady_clock;
    double secs = std::chrono::duration<double>(clk::now() - _t0).count();

    float r = 0.5f + 0.5f * std::sin(secs * 1.0);
    float g = 0.5f + 0.5f * std::sin(secs * 1.7 + 2.0);
    float b = 0.5f + 0.5f * std::sin(secs * 2.3 + 4.0);

    glViewport(0, 0, get_allocated_width(), get_allocated_height());
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return true;
}

bool GLWidget::on_tick()
{
    queue_render();
    return true; 
}

MainWindow::MainWindow()
{
    set_title("Test");
    set_default_size(1024, 600);

    m_left.set_margin(12);
    m_left.set_spacing(8);

    m_label.set_text("Controls / UI");
    m_left.append(m_label);

    m_button1.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_button1_clicked));
    m_left.append(m_button1);

    m_button2.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_button2_clicked));
    m_left.append(m_button2);

    m_left_frame.set_child(m_left);
    m_left_frame.set_size_request(320, -1);

    m_right_frame.set_child(m_glarea);

    m_paned.set_start_child(m_left_frame);
    m_paned.set_end_child(m_right_frame);
    m_paned.set_position(320);

    m_hbox.append(m_paned);
    set_child(m_hbox); 
}

void MainWindow::on_button1_clicked()
{
    m_label.set_text("Button 1 clicked");
}

void MainWindow::on_button2_clicked()
{
    m_label.set_text("Button 2 clicked");
}
