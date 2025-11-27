#include <ranges>
#include "panel.h"
#include "renderer.h"
#include "error.h"

using namespace Gtk;

SpatialPanel::SpatialPanel(MVF::SpatialHandler* handler) : handler(handler), slice_slider([this] {
    this->handler->make_current();
    static_cast<MVF::SpatialRenderer*>(this->handler->renderer)->entity.set_slice_position(slice_slider.get_value());
    slice_pos = slice_slider.get_value();
    this->handler->queue_render();
}), comp_list({}, [this]() { on_selection(); }) {
    set_label("Spatial panel");

    auto vbox = make_managed<Box>(Orientation::VERTICAL);
    auto rep_box = make_managed<Box>();
    auto rep_label = make_managed<Label>("Representation");
    rep_menu.append("None");
    rep_menu.set_active(0);
    rep_menu.set_sensitive(false);
    comp_list.set_sensitive(false);
    rep_menu.signal_changed().connect([this]() {
        if (!data) {
            return;
        }
        
        auto text = rep_menu.get_active_text();

        auto spatial_renderer = static_cast<MVF::SpatialRenderer*>(this->handler->renderer);
        if (text == "Volume" && selected_mode != Selection::VOLUME) {
            this->handler->make_current();
            if (selected_comps.size() == 2) {
                spatial_renderer->entity.set_vector_mode(selected_comps[0], selected_comps[1]);
            }
            else {
                spatial_renderer->entity.set_vector_mode(selected_comps[0], selected_comps[1], selected_comps[2]);
            }

            this->handler->queue_render();
            selected_mode = Selection::VOLUME;
        }
        else if (text == "Slice" && selected_mode != Selection::SLICE) {
            this->handler->make_current();
            spatial_renderer->entity.set_scalar_slice(selected_comps[0], selected_axis);
            spatial_renderer->entity.set_slice_position(slice_pos);
            slice_frame.set_visible(true);
            this->handler->queue_render();
            selected_mode = Selection::SLICE;
        }
        else if (text == "DVR" && selected_mode != Selection::DVR) {
            this->handler->make_current();
            spatial_renderer->entity.set_dvr(selected_comps[0]);
            this->handler->queue_render();
            selected_mode = Selection::DVR;
        }
        else if (text == "None" && selected_mode != Selection::NONE) {
            this->handler->make_current();
            spatial_renderer->entity.set_box_mode();
            this->handler->queue_render();
            selected_mode = Selection::NONE;
        }

        if (text != "Slice") {
            slice_frame.set_visible(false);
        }
    });
    rep_box->set_spacing(5);
    rep_box->set_margin(5);
    rep_box->append(*rep_label);
    rep_box->append(rep_menu);

    // Create controls for Slice representation
    slice_frame = Frame("Slice controls");
    auto radio_vbox = make_managed<Box>(Gtk::Orientation::VERTICAL);
    auto radio_label = make_managed<Label>("Normal to:");
    auto radio_box = make_managed<Box>();
    radio_box->set_halign(Gtk::Align::CENTER);
    auto r1 = make_managed<CheckButton>("X");
    auto r2 = make_managed<CheckButton>("Y");
    auto r3 = make_managed<CheckButton>("Z");
    r2->set_group(*r1);
    r3->set_group(*r1);
    r3->set_active();
    
    for (auto* btn : {r1, r2, r3}) {
        btn->signal_toggled().connect([this, btn] {
            if (btn->get_active()) {
                if(btn->get_label() == "X") {
                    selected_axis = 0;
                }
                else if (btn->get_label() == "Y") {
                    selected_axis = 1;
                }
                else {
                    selected_axis = 2;
                }

                this->handler->make_current();
                static_cast<MVF::SpatialRenderer*>(this->handler->renderer)->entity.set_slice_axis(selected_axis);
                this->handler->queue_render();
            }
        });
    }

    radio_box->append(*r1);
    radio_box->append(*r2);
    radio_box->append(*r3);
    
    radio_vbox->append(*radio_label);
    radio_vbox->append(*radio_box);
    radio_vbox->append(slice_slider);
    
    slice_frame.set_child(*radio_vbox);
    slice_frame.set_visible(false);

    auto spacer = make_managed<Box>(Orientation::VERTICAL);
    spacer->set_vexpand(true);
   
    vbox->set_spacing(5);
    vbox->append(comp_list);
    vbox->append(*rep_box);
    vbox->append(slice_frame);
    vbox->append(*spacer);

    set_child(*vbox);
    selected_mode = Selection::NONE;
}

void SpatialPanel::on_selection() {
    auto comps = comp_list.get_selected();

    // If same as previous, don't do anything
    if (comps == selected_comps) {
        return;
    }

    if (comps.size() > 3) {
        MVF::app_warn("Please set 3 or less components");
        comp_list.set_active_mask(selected_comps);
        return;
    }

    selected_comps = comps;
    
    // Clear all but first entry (None) to avoid duplicates
    while (rep_menu.get_model()->children().size() > 1) {
        for (int i = rep_menu.get_model()->children().size() - 1; i >= 1; --i) {
            rep_menu.remove_text(i);
        }
    }
    
    if (comps.size() > 1) {
        rep_menu.append("Volume");
    }
    else if (comps.size() == 1) {
        rep_menu.append("Slice");
        rep_menu.append("DVR");
    }
    rep_menu.set_active(0);
    selected_mode = Selection::NONE;
    
    handler->make_current();
    static_cast<MVF::SpatialRenderer*>(handler->renderer)->entity.set_box_mode();
    this->handler->queue_render();
}

void SpatialPanel::load_model(std::shared_ptr<MVF::VolumeData>& data) {
    this->data = data;
    
    selected_comps.clear();
    auto field_comps = this->data->scalars | std::views::transform([] (auto& field) {
        return field.first;
    }) | std::ranges::to<std::vector<std::string>>();

    comp_list.update_list(field_comps);

    // Clear all but first entry (None) to avoid duplicates
    while (rep_menu.get_model()->children().size() > 1) {
        for (int i = rep_menu.get_model()->children().size() - 1; i >= 1; --i) {
            rep_menu.remove_text(i);
        }
    }

    rep_menu.set_active(0);
    rep_menu.set_sensitive(true);
    comp_list.set_sensitive(true);

    slice_pos = 0;
    slice_slider.set_value(0);
    slice_frame.set_visible(false);

    auto spatial_renderer = static_cast<MVF::SpatialRenderer*>(handler->renderer);
    handler->make_current();
    spatial_renderer->setup_scene(data);
    handler->queue_render();
}

AttributePanel::AttributePanel(MVF::AttribHandler* handler) : handler(handler), comp_list({}, [this] { on_selection(); }) {
    set_label("Attribute panel");
    auto vbox = make_managed<Box>(Orientation::VERTICAL);

    show_attrib = CheckButton("Show attribute space");
    
    auto trait_box = make_managed<Box>();
    auto trait_label = make_managed<Label>("Trait selection");
    apply_button = Button("Apply");
    apply_button.set_hexpand(false);
    apply_button.set_halign(Gtk::Align::CENTER);
    trait_sel.append("Point");
    trait_sel.append("Range");
    trait_sel.set_active(0);
   
    trait_sel.signal_changed().connect([this] () {
        auto text = trait_sel.get_active_text();

        auto attrib_handler = static_cast<MVF::AttribHandler*>(this->handler);
        attrib_handler->set_point_trait_mode(text == "Point");
    });  

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

    if (comps == selected_comps) {
        return;
    }

    selected_comps = comps;
    auto desc = comps | std::views::transform([] (auto& name) {
        return MVF::AxisDesc{
            .comp_name = name,
            .derive = [](float val) { return val; }
        };
    }) | std::ranges::to<std::vector<MVF::AxisDesc>>();
    
    // Enable trait selection whenever components are selected (including parallel coordinates)
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
    
void FieldPanel::set_traits(const std::vector<MVF::AxisDescMeta>& attrib_comps, const std::vector<MVF::Trait>& traits) {
    handler->make_current();
    auto field_handler = static_cast<MVF::FieldRenderer*>(handler->renderer); 
    field_handler->entity.set_traits(attrib_comps, traits);
}
    
void FieldPanel::complete_set_traits() {
    enable_panel();
    auto field_handler = static_cast<MVF::FieldRenderer*>(handler->renderer); 
    field_handler->entity.complete_set_traits();
    handler->queue_render();
}

void FieldPanel::clear_traits() {
    disable_panel();

    handler->make_current();
    auto field_handler = static_cast<MVF::FieldRenderer*>(handler->renderer); 
    field_handler->entity.clear_traits();
    handler->queue_render();
}
    
void FieldPanel::enable_panel() {
    rep_menu.set_sensitive(true);
    iso_slider.set_sensitive(true);
    apply_color.set_sensitive(true);
}

void FieldPanel::disable_panel() {
    rep_menu.set_sensitive(false);
    iso_slider.set_sensitive(false);
    apply_color.set_sensitive(false);
}

FieldPanel::FieldPanel(MVF::SpatialHandler* handler) : handler(handler), iso_slider([this]() {
    static_cast<MVF::FieldRenderer*>(this->handler->renderer)->entity.set_isovalue(iso_slider.get_value());
    this->handler->queue_render();
}) {
    set_label("Feature panel");

    rep_menu.append("Isosurface");
    rep_menu.append("DVR");
    rep_menu.set_active(0);

    auto vbox = make_managed<Box>(Orientation::VERTICAL);
    auto rep_box = make_managed<Box>();
    auto rep_label = make_managed<Label>("Representation");
    rep_box->set_spacing(5);
    rep_box->set_margin(5);
    rep_box->append(*rep_label);
    rep_box->append(rep_menu);    
   
    apply_color = CheckButton("Apply colormap");

    auto spacer = make_managed<Box>(Orientation::VERTICAL);
    spacer->set_vexpand(true);
    
    vbox->append(*rep_box);
    vbox->append(iso_slider);
    vbox->append(apply_color);
    vbox->append(*spacer);

    apply_color.signal_toggled().connect([this] {
        static_cast<MVF::FieldRenderer*>(this->handler->renderer)->entity.set_apply_color(apply_color.get_active());
        this->handler->queue_render();
    });

    disable_panel();
    set_child(*vbox);
}

void FieldPanel::load_model(std::shared_ptr<MVF::VolumeData>& data) {
    auto field_renderer = static_cast<MVF::FieldRenderer*>(handler->renderer);

    handler->make_current();
    field_renderer->setup_scene(data);
    field_renderer->entity.clear_traits();
    disable_panel();
    handler->queue_render();
}
