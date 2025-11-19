#include <iostream>
#include "ui.h"
#include "vtk.h"
#include "error.h"
#include "attrib.h"

using namespace Gtk;

MainWindow* global_ui_inst = nullptr;

// This is a side channel generic function any entity can use to communicate to UI when there 
// is need for asynchronous computation
void advance_ui_clock(float fraction, bool complete) {
    if (!global_ui_inst->is_async_ui_state) {
        throw std::runtime_error("advance_ui_clock() called in synchronous mode..");
    }

    global_ui_inst->async_progress = fraction;
    if (complete) {
        global_ui_inst->disable_ui_async_state();
    }
}

MainWindow::MainWindow() : spatial_handler(&spatial_renderer), field_handler(&field_renderer), attrib_handler(&attrib_renderer), 
spatial_panel(&spatial_handler), attrib_panel(&attrib_handler), field_panel(&field_handler) {
    extern MVF::ErrorBox main_error_box;

    global_ui_inst = this;

    set_title("MVF");
    set_default_size(1024, 1024);
    main_error_box = MVF::ErrorBox(this, get_application().get());

    m_vbox.append(m_menubox);
    m_vbox.append(m_hbox);
    m_hbox.append(m_uibox);
    
    auto key_controller_spatial = Gtk::EventControllerKey::create();
    key_controller_spatial->signal_key_pressed().connect(sigc::mem_fun(spatial_handler, &MVF::SpatialHandler::on_key_pressed), false);
    add_controller(key_controller_spatial);
    
    auto key_controller_field = Gtk::EventControllerKey::create();
    key_controller_field->signal_key_pressed().connect(sigc::mem_fun(field_handler, &MVF::SpatialHandler::on_key_pressed), false);

    auto spatial_hbox = build_menu({
        {
            .tooltip_text = "Reset camera",
            .icon_filename = "assets/reset.png",
            .handler = [this] {
                spatial_handler.reset_camera();
            }
        }, 
        {
            .tooltip_text = "Switch to distance field",
            .icon_filename = "assets/toggle-on.png",
            .handler = [this, key_controller_spatial, key_controller_field] {
                pane.set_start_child(field_box);
                m_uibox.remove(spatial_panel);
                m_uibox.prepend(field_panel);
                
                if (!is_field_model_init) {
                    field_panel.load_model(data);
                }
                is_field_model_init = true;
                is_feature_space_visible = false;
                remove_controller(key_controller_spatial);
                add_controller(key_controller_field);

                if (trait_handler_pending) {
                    attrib_panel.set_button_active();
                    trait_handler_pending = false;
                }

                if (clear_handler_pending) {
                    field_panel.clear_traits();
                    clear_handler_pending = false;
                }
            }
        }
    });
    
    attrib_hbox = build_menu({
        {
            .tooltip_text = "Clear traits",
            .icon_filename = "assets/reset.png",
            .handler = [this] {
                if (is_async_ui_state) {
                    return;
                }

                attrib_renderer.clear_traits();
                attrib_handler.queue_render();
                if (is_feature_space_visible) {
                    clear_handler_pending = true;
                }
                else {
                    field_panel.clear_traits();
                }

                attrib_panel.set_button_inactive();
                trait_handler_pending = false;
            }
        }
    });

    show_plot_button = build_button("Show plot", "assets/show.png", [this] {
        if (disable_set_plot) {
            return;
        }
        
        attrib_handler.make_current();
        attrib_renderer.enable_plot(true);
        attrib_handler.queue_render();
        set_plot(false);
    });  

    hide_plot_button = build_button("Hide plot", "assets/hide.png", [this] {
        if (disable_set_plot) {
            return;
        }
        
        attrib_handler.make_current();
        attrib_renderer.enable_plot(false);
        attrib_handler.queue_render();
        set_plot(true); 
    });
    
    attrib_hbox->append(*show_plot_button);
    attrib_hbox->append(*hide_plot_button);
    set_plot(true);
    
    auto field_hbox = build_menu({
        {
            .tooltip_text = "Reset camera",
            .icon_filename = "assets/reset.png",
            .handler = [this] {
                field_handler.reset_camera();
            }
        },
        {
            .tooltip_text = "Switch to domain space",
            .icon_filename = "assets/toggle-off.png",
            .handler = [this, key_controller_spatial, key_controller_field] {
                if (is_async_ui_state) {
                    return;
                }
                
                pane.set_start_child(spatial_box); 
                m_uibox.remove(field_panel);
                m_uibox.prepend(spatial_panel);

                if (!is_spatial_model_init) {
                    spatial_panel.load_model(data);
                }
                is_spatial_model_init = true;
                is_feature_space_visible = true;
                remove_controller(key_controller_field);
                add_controller(key_controller_spatial);

                if (attrib_panel.state.apply_button_state) {
                    attrib_panel.set_button_inactive();
                    trait_handler_pending = true;
                }
            }
        }
    });

    spatial_box.append(*spatial_hbox);
    spatial_box.append(spatial_handler);
    attrib_box.append(*attrib_hbox);
    attrib_box.append(attrib_handler);
    field_box.append(*field_hbox);
    field_box.append(field_handler);
    spatial_box.set_hexpand(true);
    attrib_box.set_hexpand(true);
    field_box.set_hexpand(true);
    pane.set_start_child(spatial_box);
    pane.set_css_classes({"draw-board"});
   
    pane.signal_realize().connect([this]() {
        // Wait for one main loop iteration so allocation is ready
        Glib::signal_idle().connect_once([this] () {
            int width = pane.get_allocated_width();
            if (width > 0)
                pane.set_position(width / 2);
        });
    });
    m_hbox.append(pane);

    spatial_panel.set_vexpand(true);
    attrib_panel.set_vexpand(true);
    m_uibox.append(spatial_panel);
    m_uibox.append(attrib_panel);

    auto file_open_btn = make_managed<Button>();
    auto file_open_icon = make_managed<Image>("assets/file-open.png"); 
    file_open_btn->set_child(*file_open_icon);
    file_open_btn->set_tooltip_text("Open vtk file");
    file_open_btn->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_file_open));
    m_menubox.append(*file_open_btn);

    m_uibox.append(progress_bar);
    
    m_uibox.set_css_classes({"main-vbox"});
    m_menubox.set_css_classes({"main-vbox"});
    file_open_btn->set_css_classes({"menu-button"});
    file_open_btn->set_has_frame(false);
    set_child(m_vbox);
    
    set_focusable(true); 
    
    attrib_panel.show_attrib.signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::toggle_attrib_space));

    auto mouse_click = GestureClick::create();
    mouse_click->signal_pressed().connect([this] (int, double, double) {
        if (attrib_handler.handle_traits) {
            if (is_feature_space_visible) {
                trait_handler_pending = true;
            }
            else {
                attrib_panel.set_button_active();
            }
            attrib_handler.handle_traits = false;
        }
    });

    attrib_panel.apply_button.signal_clicked().connect([this] {
        if (is_async_ui_state || file_loader_conn.connected()) {
#ifdef MVF_DEBUG
            std::cout << "Timer lock held..." << std::endl;
#endif
            return;
        }
        attrib_panel.set_button_inactive();
        trait_handler_pending = false;

        auto [comp, traits] = attrib_renderer.get_traits();
        enable_ui_async_state();
        field_panel.set_traits(comp, traits);
        attrib_panel.comp_list.set_sensitive(false);
    });

    attrib_panel.comp_list.set_secondary_handler([this] {
        if (!attrib_panel.handle_changed_selection) {
            return;
        }
        
        attrib_panel.set_button_inactive();
        trait_handler_pending = false;

        if (is_feature_space_visible) {
            clear_handler_pending = true;
        }
        else {
            field_panel.clear_traits();
        }

        disable_set_plot = attrib_panel.comp_list.get_selected().size() == 0;

        set_plot(true);
        attrib_panel.handle_changed_selection = false;
    });
    
    add_controller(mouse_click);

    signal_close_request().connect(sigc::mem_fun(*this, &MainWindow::on_window_close), false);
}

void MainWindow::set_plot(bool show) {
    show_plot_button->set_visible(show);
    hide_plot_button->set_visible(!show);
    show_plot = show;
}
    
bool MainWindow::generic_async_handler() {
    if(!is_async_ui_state) {
        field_panel.complete_set_traits();
        attrib_panel.comp_list.set_sensitive(true);
        return false;
    }

    progress_bar.set_fraction(async_progress);
    return true;
}
    
void MainWindow::enable_ui_async_state() {
    if (file_loader_conn.connected()) {
        throw std::runtime_error("enable_ui_async_state() called when file_loader_conn is connected...");
    }
    progress_bar.show();
    progress_bar.set_fraction(0);
    async_progress = 0;
    file_loader_conn = Glib::signal_timeout().connect(sigc::mem_fun(*this, &MainWindow::generic_async_handler), 16);
    is_async_ui_state = true;
}

void MainWindow::disable_ui_async_state() {
    if (!is_async_ui_state) {
        return;
    }
    progress_bar.hide();
    is_async_ui_state = false;
}
    
void MainWindow::toggle_attrib_space() {
    if (is_attrib_space_visible) {
        pane.unset_end_child();
        attrib_panel.disable_panel();
    }
    else {
        pane.set_end_child(attrib_box);
        attrib_panel.enable_panel();
    }

    is_attrib_space_visible = !is_attrib_space_visible;
}

Gtk::Button* MainWindow::build_button(const std::string& tooltip_text, const std::string& icon_filename, std::function<void ()> handler) {
    auto button = make_managed<Button>();
    auto icon = make_managed<Image>(icon_filename);
    button->set_child(*icon);
    button->set_tooltip_text(tooltip_text);
    button->set_has_frame(false);
    
    button->signal_clicked().connect(handler);

    return button;
}

Gtk::Box* MainWindow::build_menu(const std::vector<ButtonDescriptor>& desc) {
    auto hbox = make_managed<Box>();
    hbox->set_spacing(5);
    hbox->set_css_classes({"main-vbox"});
    hbox->set_hexpand(true);

    for (auto& val: desc) {
        hbox->append(*build_button(val.tooltip_text, val.icon_filename, val.handler));
    }

    return hbox;
}

bool MainWindow::on_window_close() {
    if (file_loader_conn.connected()) {
        loader->cancel_io();
        loader->complete();
        field_renderer.entity.cancel_dist_computation();
    }

    return false;
}

void MainWindow::on_file_open() {
    if (file_loader_conn.connected()) {
#ifdef MVF_DEBUG
        std::cout << "Timer lock could not be obtained..." << std::endl;
#endif
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

        data = loader->data;
        // We tell MainWindow to hold off on loading the scene if that view is hidden
        // When view is realized later, the UI calls view's load_model function
        if (is_feature_space_visible) {
            spatial_panel.load_model(loader->data);
            is_field_model_init = false;
        }
        else {
            field_panel.load_model(loader->data);
            is_spatial_model_init = false;
        }

        attrib_panel.load_model(loader->data);
        trait_handler_pending = false;
        clear_handler_pending = false;

        return false;
    }

    return true;
}