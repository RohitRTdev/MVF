#pragma once

#include <gtkmm.h>
#include <memory>
#include "handler.h"
#include "vtk.h"

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
    Selection selected_mode;
};

class AttributePanel : public MVFPanel {
public:
    AttributePanel(MVF::AttribHandler* handler);
    void load_model(std::shared_ptr<MVF::VolumeData>& data);

private:
    MVF::AttribHandler* handler;
    Gtk::ComboBoxText dim_menu;
    size_t max_dim = 0;
};