#include <iostream>
#include "ui.h"
#include "vtk.h"
#include "error.h"

using namespace Gtk;

MainWindow::MainWindow() : gl_handler(this) {
    extern MVF::ErrorBox main_error_box;
    
    set_title("MVF");
    set_default_size(1024, 1024);
    main_error_box = MVF::ErrorBox(this, get_application().get());

    m_vbox.append(m_menubox);
    m_vbox.append(m_hbox);
    m_hbox.append(m_uibox);
    m_hbox.append(gl_handler);

    m_label.set_text("Controls / UI");
    m_uibox.append(m_label);

    m_button1.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_button1_clicked));
    m_button1.set_label("Load");
    m_uibox.append(m_button1);

    auto file_open_btn = make_managed<Button>();
    auto file_open_icon = make_managed<Image>("assets/file-open.png"); 
    file_open_btn->set_child(*file_open_icon);
    file_open_btn->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_file_open));
    m_menubox.append(*file_open_btn);

    auto spacer = make_managed<Gtk::Box>();
    spacer->set_vexpand(true);  
    m_uibox.append(*spacer);
    m_uibox.append(progress_bar);
    
    m_uibox.set_css_classes({"main-vbox"});
    m_menubox.set_css_classes({"main-vbox"});
    file_open_btn->set_css_classes({"menu-button"});
    file_open_btn->set_has_frame(false);
    m_button1.set_css_classes({"ui-button"});
    set_child(m_vbox);
    
    set_focusable(true); 
    auto key_controller = Gtk::EventControllerKey::create();
    key_controller->signal_key_pressed().connect(sigc::mem_fun(gl_handler, &MVF::RenderHandler::on_key_pressed), false);
    add_controller(key_controller);

    signal_close_request().connect(sigc::mem_fun(*this, &MainWindow::on_window_close), false);
}
    
OverlayProgressBar::OverlayProgressBar() : Gtk::Box(Gtk::Orientation::VERTICAL, 0) {
    auto overlay = Gtk::make_managed<Gtk::Overlay>();

    progress_bar.set_show_text(false);
    progress_bar.set_fraction(0.0);
    progress_bar.set_css_classes({"thick-bar"});
    progress_bar.set_sensitive(false);
    progress_bar.set_opacity(0.0);
    overlay->set_child(progress_bar);

    label_progress.set_text("0%");
    label_progress.set_css_classes({"progress-label"});
    label_progress.set_halign(Gtk::Align::CENTER);
    label_progress.set_valign(Gtk::Align::CENTER);
    label_progress.set_opacity(0.0);
    overlay->add_overlay(label_progress);

    append(*overlay);
}

void OverlayProgressBar::show() {
    progress_bar.set_opacity(1.0);
    label_progress.set_opacity(1.0);
    progress_bar.set_sensitive(true);
}

void OverlayProgressBar::hide() {
    progress_bar.set_opacity(0.0);
    label_progress.set_opacity(0.0);
    progress_bar.set_sensitive(false);
}

void OverlayProgressBar::set_fraction(double frac) {
    progress_bar.set_fraction(frac);
    int pct = static_cast<int>(frac * 100);
    label_progress.set_text(std::to_string(pct) + "%");
}

bool MainWindow::on_window_close() {
    if (file_loader_conn.connected()) {
        loader->cancel_io();
        loader->complete();
    }

    return false;
}

void MainWindow::on_file_open() {
    if (file_loader_conn.connected()) {
        std::cout << "File loading in progress..." << std::endl;
        return;
    }
    
    auto dialog = FileChooserNative::create(
        "Select a vtk file",
        *this,
        Gtk::FileChooser::Action::OPEN,
        "_Open",
        "_Cancel"
    );

    auto vtk_filter = Gtk::FileFilter::create();
    vtk_filter->set_name("VTK files");
    vtk_filter->add_pattern("*.vtk"); 
    dialog->add_filter(vtk_filter);

    dialog->show();

    dialog->signal_response().connect([dialog, this](int response) {
        if (response == Gtk::ResponseType::ACCEPT) {
            auto filename = dialog->get_file()->get_path();
           
            loader = MVF::open_vtk_async(filename);
            if (loader->read_failed) {
                return;
            }
            
            vtk_filename = filename;
            progress_bar.show();
            file_loader_conn = Glib::signal_timeout().connect(sigc::mem_fun(*this, &MainWindow::file_load_handler), 16);
            loader->load();
        }
    });
}

bool MainWindow::file_load_handler() {
    if (loader->read_failed) {
        MVF::app_warn("Invalid file format");
        loader->complete();
        progress_bar.hide();
        return false;
    }

    auto bytes_read = loader->num_bytes_read.load(std::memory_order_relaxed);
    auto fraction = static_cast<double>(bytes_read) / loader->total_bytes;
    progress_bar.set_fraction(fraction);
    
    if (bytes_read == loader->total_bytes) {
        loader->complete();
        progress_bar.hide();
#ifdef MVF_DEBUG
        std::cout << "Loaded VTK: " << loader->data->nx << " x " << loader->data->ny << " x " << loader->data->nz << " with fields:" << std::endl;

        for (auto& [key, val]: loader->data->scalars) {
            std::cout << key << " -> " << std::get<0>(val).size() << ", " << std::get<1>(val) << std::endl;
        }

        std::cout << "Origin: " << loader->data->origin.x << ',' << loader->data->origin.y << ',' << loader->data->origin.z << std::endl;
        std::cout << "Spacing: " << loader->data->spacing.x << ',' << loader->data->spacing.y << ',' << loader->data->spacing.z << std::endl;
#endif           
        gl_handler.make_current();
        renderer.setup_scene(loader->data);
        loaded_scene = true;
        gl_handler.queue_render();

        return false;
    }

    return true;
}

void MainWindow::on_button1_clicked() {
    m_label.set_text("Vector arrow glyph");
}