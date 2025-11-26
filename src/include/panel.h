#pragma once

#include <gtkmm.h>
#include <memory>
#include "handler.h"
#include "vtk.h"
#include "attrib.h"
#include "widgets.h"

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
        VOLUME,
        SLICE,
        DVR
    };

    MVF::SpatialHandler* handler;
    Gtk::ComboBoxText rep_menu;
    Gtk::Frame slice_frame;
    Slider slice_slider;
    Selection selected_mode;
    int selected_axis = 2;
    float slice_pos = 0;
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
        bool trait_sel_state;
        bool apply_button_state;
    };

    MVF::AttribHandler* handler;
    Gtk::ComboBoxText trait_sel;
    MultiSelectCombo comp_list;
    Gtk::CheckButton show_attrib;
    Gtk::Button apply_button;
    bool handle_changed_selection = false;
    std::vector<std::string> selected_comps;
    
    PanelState state = {false, false};
    bool is_panel_enabled = false;

    void change_state(const PanelState& state);
    void on_selection();
};

class FieldPanel : public MVFPanel {
public: 
    FieldPanel(MVF::SpatialHandler* handler);
    void load_model(std::shared_ptr<MVF::VolumeData>& data);
    void clear_traits();
    void set_traits(const std::vector<MVF::AxisDescMeta>& attrib_comps, const std::vector<MVF::Trait>& traits);
    void complete_set_traits();
    void enable_panel();
    void disable_panel();
private:

    MVF::SpatialHandler* handler;
    Gtk::ComboBoxText rep_menu;
    Slider iso_slider;
    Gtk::CheckButton apply_color;
};