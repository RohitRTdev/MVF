#pragma once

#include <gtkmm.h>
#include "handler.h"
#include "vtk.h"
#include "attrib.h"

class MainWindow;

class MVFPanel : public Gtk::Frame {
protected:
    std::shared_ptr<MVF::VolumeData> data;
};

class SpatialPanel : public MVFPanel {
public:
    SpatialPanel(MVF::SpatialHandler* handler);
    void load_model(std::shared_ptr<MVF::VolumeData>& data);

private:
    enum class Selection {
        NONE,
        VOLUME
    };

    MVF::SpatialHandler* handler;
    Gtk::ComboBoxText rep_menu;
    Selection selected_mode;
};

class AttributePanel : public MVFPanel {
public:
    AttributePanel(MVF::AttribHandler* handler);
    void load_model(std::shared_ptr<MVF::VolumeData>& data);
    void disable_panel();
    void enable_panel();
    void set_button_active();
    void set_button_inactive();

    friend MainWindow;
private:
    
    struct PanelState {
        bool dim_menu_state;
        bool trait_sel_state;
        bool apply_button_state;
    };

    MVF::AttribHandler* handler;
    Gtk::ComboBoxText dim_menu, trait_sel;
    Gtk::CheckButton show_attrib;
    Gtk::Button apply_button;
    size_t max_dim = 0;
    size_t prev_value = 0;
    bool handle_changed_value = false;

    PanelState state = {false, false, false};

    void change_state(const PanelState& state);
};

class FieldPanel : public MVFPanel {
public: 
    FieldPanel(MVF::SpatialHandler* handler);
    void load_model(std::shared_ptr<MVF::VolumeData>& data);
    void clear_traits();
    void set_traits(const std::vector<MVF::AxisDesc>& attrib_comps, const std::vector<MVF::Trait>& traits);
    void enable_panel();
    void disable_panel();
private:

    MVF::SpatialHandler* handler;
    Gtk::ComboBoxText rep_menu;
    Gtk::Scale iso_slider;
};