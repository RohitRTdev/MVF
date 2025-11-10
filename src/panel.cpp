#include <ranges>
#include "panel.h"
#include "renderer.h"
#include "error.h"

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

AttributePanel::AttributePanel(MVF::AttribHandler* handler) : handler(handler), comp_list({}, [this]() { on_selection(); }) {
    set_label("Attribute panel");
    auto vbox = make_managed<Box>(Orientation::VERTICAL);

    show_attrib = CheckButton("Show attribute space");
    
    auto trait_box = make_managed<Box>();
    auto trait_label = make_managed<Label>("Trait selection");
    apply_button = Button("Apply");
    apply_button.set_hexpand(false);
    trait_sel.append("Point");
    trait_sel.append("Range");
    trait_sel.set_active(0);

    comp_list.set_sensitive(false);
    trait_sel.set_sensitive(false);
    apply_button.set_sensitive(false);
    
    trait_box->set_spacing(5);
    trait_box->set_margin(5);
    trait_box->append(*trait_label);
    trait_box->append(trait_sel);
    
    auto spacer = make_managed<Box>(Orientation::VERTICAL);
    spacer->set_vexpand(true);
   
    vbox->set_spacing(5);
    vbox->append(show_attrib);
    vbox->append(comp_list);
    vbox->append(*trait_box);
    vbox->append(apply_button);
    vbox->append(*spacer);

    set_child(*vbox);
}

void AttributePanel::on_selection() {
    auto comps = comp_list.get_selected();

    // If same as previous, don't do anything
    if (comps == selected_comps) {
        return;
    }

    if (comps.size() > 2) {
        MVF::app_warn("Please set only 2 or less components");
        comp_list.set_active_mask(selected_comps);
        return;
    }

    selected_comps = comps;
    auto desc = comps | std::views::transform([] (auto& name) {
        return MVF::AxisDesc{
            .comp_name = name,
            .derive = [](float val) { return val; }
        };
    }) | std::ranges::to<std::vector<MVF::AxisDesc>>();
    
    change_state({comps.size() > 0, false});
    handle_changed_selection = true;

    handler->set_field_info(desc);
    handler->queue_render();
}
    
void AttributePanel::disable_panel() {
    state.trait_sel_state = trait_sel.get_sensitive();
    state.apply_button_state = apply_button.get_sensitive();
    trait_sel.set_sensitive(false);
    apply_button.set_sensitive(false);
}

void AttributePanel::enable_panel() {
    change_state(state);
}

void AttributePanel::change_state(const PanelState& state) {
    this->state = state;
    
    if (show_attrib.get_active()) {
        trait_sel.set_sensitive(state.trait_sel_state);
        apply_button.set_sensitive(state.apply_button_state);
    }
}
    
void AttributePanel::set_button_active() {
    state.apply_button_state = true;
    apply_button.set_sensitive(true);
}

void AttributePanel::set_button_inactive() {
    state.apply_button_state = false;
    apply_button.set_sensitive(false);
}

void AttributePanel::load_model(std::shared_ptr<MVF::VolumeData>& data) {
    this->data = data;

    selected_comps.clear();
    auto field_comps = this->data->scalars | std::views::transform([] (auto& field) {
        return field.first;
    }) | std::ranges::to<std::vector<std::string>>();

    comp_list.update_list(field_comps);
   
    comp_list.set_sensitive(true);
    change_state({true, false});

    static_cast<MVF::AttribRenderer*>(handler->renderer)->set_field_data(this->data);
    handler->clear_field_info();
    handler->queue_render();
}
    
void FieldPanel::set_traits(const std::vector<MVF::AxisDesc>& attrib_comps, const std::vector<MVF::Trait>& traits) {
    rep_menu.set_sensitive(true);
    iso_slider.set_sensitive(true);

    handler->make_current();
    auto field_handler = static_cast<MVF::FieldRenderer*>(handler->renderer); 
    field_handler->entity.set_traits(attrib_comps, traits);
    handler->queue_render();
}

void FieldPanel::clear_traits() {
    rep_menu.set_sensitive(false);
    iso_slider.set_sensitive(false);

    handler->make_current();
    auto field_handler = static_cast<MVF::FieldRenderer*>(handler->renderer); 
    field_handler->entity.clear_traits();
    handler->queue_render();
}
    
void FieldPanel::enable_panel() {
    rep_menu.set_sensitive(true);
    iso_slider.set_sensitive(true);
}

void FieldPanel::disable_panel() {
    rep_menu.set_sensitive(false);
    iso_slider.set_sensitive(false);
}

FieldPanel::FieldPanel(MVF::SpatialHandler* handler) : handler(handler) {
    set_label("Feature panel");

    rep_menu.append("Isosurface");
    rep_menu.append("DVR");
    rep_menu.set_active(0);
    rep_menu.set_sensitive(false);

    auto vbox = make_managed<Box>(Orientation::VERTICAL);
    auto rep_box = make_managed<Box>();
    auto rep_label = make_managed<Label>("Representation");
    rep_box->set_spacing(5);
    rep_box->set_margin(5);
    rep_box->append(*rep_label);
    rep_box->append(rep_menu);
    
    // Create a horizontal slider
    auto adjustment = Gtk::Adjustment::create(0.0, 0.0, 1.0, 0.01, 0.1);

    iso_slider.set_adjustment(adjustment);
    iso_slider.set_value_pos(Gtk::PositionType::LEFT);
    iso_slider.set_digits(2);
    iso_slider.set_draw_value(true); 
    iso_slider.set_sensitive(false);

    iso_slider.signal_value_changed().connect([this]() {
        static_cast<MVF::FieldRenderer*>(this->handler->renderer)->entity.set_isovalue(iso_slider.get_value());
        this->handler->queue_render();
    });

    auto spacer = make_managed<Box>(Orientation::VERTICAL);
    spacer->set_vexpand(true);
    
    vbox->append(*rep_box);
    vbox->append(iso_slider);
    vbox->append(*spacer);
    set_child(*vbox);
}

void FieldPanel::load_model(std::shared_ptr<MVF::VolumeData>& data) {
    auto field_renderer = static_cast<MVF::FieldRenderer*>(handler->renderer);

    handler->make_current();
    field_renderer->setup_scene(data);
    field_renderer->entity.clear_traits();
    handler->queue_render();
}
