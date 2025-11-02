#pragma once

#include <gtkmm.h>
#include "handler.h"
#include "vtk.h"

class MVFPanel : public Gtk::Frame {
public:
    MVFPanel(MVF::RenderHandler* handler);

protected:
    MVF::RenderHandler* handler;
    std::shared_ptr<MVF::VolumeData> data;
};

class SpatialPanel : public MVFPanel {
public:
    SpatialPanel(MVF::RenderHandler* handler);
    void load_model(std::shared_ptr<MVF::VolumeData>& data);

private:
    enum class Selection {
        NONE,
        VOLUME
    };

    Gtk::ComboBoxText rep_menu;
    Selection selected_mode;
};

class AttributePanel : public MVFPanel {
public:
    AttributePanel(MVF::RenderHandler* handler);
    void load_model(std::shared_ptr<MVF::VolumeData>& data);

private:
    Gtk::ComboBoxText dim_menu;
    size_t max_dim = 0;
};