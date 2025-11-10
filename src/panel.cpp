#include "panel.h"
#include "renderer.h"

using namespace Gtk;

SpatialPanel::SpatialPanel(MVF::SpatialHandler* handler) : handler(handler) {
    set_label("Spatial panel");

    auto vbox = make_managed<Box>(Orientation::VERTICAL);
    auto rep_box = make_managed<Box>();
    auto rep_label = make_managed<Label>("Representation");
    rep_menu.append("None");
    rep_menu.set_active(0);
    rep_menu.signal_changed().connect([this]() {
        if (!data) {
            return;
        }
        
        auto text = rep_menu.get_active_text();

        auto spatial_renderer = static_cast<MVF::SpatialRenderer*>(this->handler->renderer);
        if (text == "Volume" && selected_mode != Selection::VOLUME) {
            this->handler->make_current();
            std::vector<std::string> keys;
            for (auto& [key, _]: data->scalars) {
                keys.push_back(key);
            }
            if (keys.size() >= 3) {
                spatial_renderer->entity.set_vector_mode(keys[0], keys[1], keys[2]);
            }
            this->handler->queue_render();
            selected_mode = Selection::VOLUME;
        }
        else if (text == "Slice" && selected_mode != Selection::SLICE) {
            this->handler->make_current();
            std::string key;
            for (auto& [k,_]: data->scalars) { key = k; break; }
            spatial_renderer->entity.set_scalar_slice(key, 2);
            this->handler->queue_render();
            selected_mode = Selection::SLICE;
        }
        else if (text == "DVR" && selected_mode != Selection::DVR) {
            this->handler->make_current();
            std::string key;
            for (auto& [k,_]: data->scalars) { key = k; break; }
            spatial_renderer->entity.set_dvr(key);
            this->handler->queue_render();
            selected_mode = Selection::DVR;
        }
        else if (text == "None" && selected_mode != Selection::NONE) {
            this->handler->make_current();
            spatial_renderer->setup_scene(data);
            this->handler->queue_render();
            selected_mode = Selection::NONE;
        }
    });
    rep_box->set_spacing(5);
    rep_box->set_margin(5);
    rep_box->append(*rep_label);
    rep_box->append(rep_menu);
    
    auto spacer = make_managed<Box>(Orientation::VERTICAL);
    spacer->set_vexpand(true);
    
    vbox->append(*rep_box);
    vbox->append(*spacer);

    set_child(*vbox);
    selected_mode = Selection::NONE;
}

void SpatialPanel::load_model(std::shared_ptr<MVF::VolumeData>& data) {
    this->data = data;
    // Clear all but first entry (None) to avoid duplicates
    while (rep_menu.get_model()->children().size() > 1) {
        for (int i = rep_menu.get_model()->children().size() - 1; i >= 1; --i) {
            rep_menu.remove_text(i);
        }
    }

    if (data->scalars.size() >= 3) {
        rep_menu.append("Volume");
    }
    if (data->scalars.size() >= 1) {
        rep_menu.append("Slice");
        rep_menu.append("DVR");
    }
    rep_menu.set_active(0);

    auto spatial_renderer = static_cast<MVF::SpatialRenderer*>(handler->renderer);
    handler->make_current();
    spatial_renderer->setup_scene(data);
    handler->queue_render();
}

AttributePanel::AttributePanel(MVF::AttribHandler* handler) : handler(handler) {
    set_label("Attribute panel");
    auto vbox = make_managed<Box>(Orientation::VERTICAL);
    auto dim_box = make_managed<Box>();
    auto dim_label = make_managed<Label>("Dimensions");
    dim_menu.append("0");
    dim_menu.set_active(0);
    dim_menu.signal_changed().connect([this]() {
        if (!this->data) {
            return;
        }

        size_t value = std::stoul(dim_menu.get_active_text());
        this->handler->make_current();
        std::vector<MVF::AxisDesc> desc;
        size_t count = 0;
        for (auto& [name, _] : this->data->scalars) {
            if (count >= value) break;
            // display name same as component name for now
            desc.push_back(MVF::AxisDesc{ name, name, [](float v){ return v; } });
            ++count;
        }
        this->handler->set_field_info(desc);
        this->handler->queue_render();
    });
    dim_box->set_spacing(5);
    dim_box->set_margin(5);
    dim_box->append(*dim_label);
    dim_box->append(dim_menu);
    
    auto spacer = make_managed<Box>(Orientation::VERTICAL);
    spacer->set_vexpand(true);
    
    vbox->append(*dim_box);
    vbox->append(*spacer);

    set_child(*vbox);
}

void AttributePanel::load_model(std::shared_ptr<MVF::VolumeData>& data) {
    this->data = data;
    dim_menu.set_active(0);
    for (size_t i = max_dim; i > 0; i--) {
        dim_menu.remove_text(i);
    }
    max_dim = std::min(this->data->scalars.size(), (size_t)2);
    for (size_t i = 1; i <= max_dim; i++) {
        dim_menu.append(std::to_string(i));
    }
    handler->make_current();
    static_cast<MVF::AttribRenderer*>(handler->renderer)->set_field_data(this->data);
    handler->queue_render();
}
