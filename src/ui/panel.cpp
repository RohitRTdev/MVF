#include <ranges>
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
    rep_menu.set_sensitive(false);
    rep_menu.signal_changed().connect([this]() {
        if (!data) {
            return;
        }
        
        auto text = rep_menu.get_active_text();

        auto spatial_renderer = static_cast<MVF::SpatialRenderer*>(this->handler->renderer);
        if (text == "Volume" && selected_mode != Selection::VOLUME) {
            this->handler->make_current();
            std::vector<MVF::AxisDesc> desc(1);
            std::vector<std::string> keys;
            for (auto& [key, _]: data->scalars) {
                keys.push_back(key);
            }
            spatial_renderer->entity.set_vector_mode(keys[0], keys[1], keys[2]);
            this->handler->queue_render();
            desc.push_back(MVF::AxisDesc{});

            selected_mode = Selection::VOLUME;
        }
        else if (text == "None" && selected_mode != Selection::NONE) {
            this->handler->make_current();
            spatial_renderer->entity.set_box_mode();
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
    bool append_option = false;
    rep_menu.set_active(0);
    selected_mode = Selection::NONE;
    if (!this->data || this->data->scalars.size() < 3) {
        append_option = true;
    }

    this->data = data;
    if (append_option) {
        if (this->data->scalars.size() >= 3) {
            rep_menu.append("Volume");
        }
    }
    else {
        if (this->data->scalars.size() < 3) {
            rep_menu.remove_text(1);
        }
    }
    rep_menu.set_sensitive(true);
    
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
    auto trait_box = make_managed<Box>();
    auto trait_label = make_managed<Label>("Trait selection");
    trait_sel.append("Point");
    trait_sel.append("Range");
    dim_menu.append("0");
    dim_menu.set_active(0);
    trait_sel.set_active(0);
    dim_menu.set_sensitive(false);
    trait_sel.set_sensitive(false);
    dim_menu.signal_changed().connect([this]() {
        if (!this->data) {
            return;
        }

        size_t value = std::stoi(dim_menu.get_active_text());
        
        this->handler->make_current();
        
        auto desc = this->data->scalars | std::views::take(value) | std::views::transform([](auto& pair) {
            auto& [name, _] = pair;
            return MVF::AxisDesc{
                .comp_name = name,
                .derive = [](float val) { return val; }
            };
        }) | std::ranges::to<std::vector>();            

        trait_sel.set_sensitive(value > 0);

        this->handler->set_field_info(desc);
        this->handler->queue_render();
    });
    dim_box->set_spacing(5);
    dim_box->set_margin(5);
    trait_box->set_spacing(5);
    trait_box->set_margin(5);
    dim_box->append(*dim_label);
    dim_box->append(dim_menu);
    trait_box->append(*trait_label);
    trait_box->append(trait_sel);
    
    auto spacer = make_managed<Box>(Orientation::VERTICAL);
    spacer->set_vexpand(true);
    
    vbox->append(*dim_box);
    vbox->append(*trait_box);
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

    dim_menu.set_sensitive(true);
    trait_sel.set_sensitive(false);
    handler->make_current();
    static_cast<MVF::AttribRenderer*>(handler->renderer)->set_field_data(this->data);
    handler->queue_render();
}
