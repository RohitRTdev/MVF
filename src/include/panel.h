#pragma once

#include <gtkmm.h>
#include "handler.h"
#include "vtk.h"

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

    friend MainWindow;
private:
    
    struct PanelState {
        bool dim_menu_state;
        bool trait_sel_state;
    };

    MVF::AttribHandler* handler;
    Gtk::ComboBoxText dim_menu, trait_sel;
    Gtk::CheckButton show_attrib;
    size_t max_dim = 0;
    PanelState state = {false, false};
};