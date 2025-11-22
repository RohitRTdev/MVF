#include <iostream>
#include <format>
#include "widgets.h"

OverlayProgressBar::OverlayProgressBar() : Gtk::Box(Gtk::Orientation::VERTICAL, 0) {
    auto overlay = Gtk::make_managed<Gtk::Overlay>();

    progress_bar.set_show_text(false);
    progress_bar.set_fraction(0.0);
    progress_bar.set_css_classes({"thick-bar"});
    progress_bar.set_sensitive(false);
    progress_bar.set_opacity(0.0);
    overlay->set_child(progress_bar);

    label_progress.set_text("0%");
    label_progress.set_css_classes({"progress-label"});
    label_progress.set_halign(Gtk::Align::CENTER);
    label_progress.set_valign(Gtk::Align::CENTER);
    label_progress.set_opacity(0.0);
    overlay->add_overlay(label_progress);

    append(*overlay);
}

void OverlayProgressBar::show() {
    progress_bar.set_opacity(1.0);
    label_progress.set_opacity(1.0);
    progress_bar.set_sensitive(true);
}

void OverlayProgressBar::hide() {
    progress_bar.set_opacity(0.0);
    label_progress.set_opacity(0.0);
    progress_bar.set_sensitive(false);
}

void OverlayProgressBar::set_fraction(double frac) {
    progress_bar.set_fraction(frac);
    int pct = static_cast<int>(frac * 100);
    label_progress.set_text(std::to_string(pct) + "%");
}

HoverOverlay::HoverOverlay() {
    set_has_arrow(false);
    set_autohide(false); 
    set_can_target(false);
    set_can_focus(false);
    set_position(Gtk::PositionType::RIGHT);
    set_child(label);
}

void HoverOverlay::show_at(int x, int y, const std::string& text) {
    Gdk::Rectangle rect;
    get_pointing_to(rect);
   
    int dx = x - rect.get_x(), dy = y - rect.get_y();
    if (std::abs(dx) > std::abs(dy)) {
        if (dx > 0) {
            set_position(Gtk::PositionType::LEFT);
            set_offset(-20, 0);
        }
        else {
            set_position(Gtk::PositionType::RIGHT);
            set_offset(20, 0);
        }
    }
    else {
        if (dy > 0) {
            set_position(Gtk::PositionType::TOP);
            set_offset(0, -20);
        }
        else {
            set_position(Gtk::PositionType::BOTTOM);
            set_offset(0, 20);
        }
    }
    set_pointing_to(Gdk::Rectangle{x, y, 1, 1});
    label.set_text(text);
    popup();
}

void HoverOverlay::hide_now() {
    popdown();
}

MultiSelectCombo::MultiSelectCombo(const std::vector<std::string>& options, std::function<void()> fn) : 
Gtk::Box(Gtk::Orientation::VERTICAL), handler(fn) {
    set_spacing(4);

    main_button.set_label("Select Components ▼");
    main_button.set_hexpand(false);
    main_button.set_halign(Gtk::Align::CENTER);
    main_button.signal_clicked().connect(sigc::mem_fun(*this, &MultiSelectCombo::on_button_clicked));
    append(main_button);

    popover.set_has_arrow(false);
    popover.set_parent(main_button);

    vbox = Gtk::Box(Gtk::Orientation::VERTICAL);
    vbox.set_margin(8);
    vbox.set_spacing(4);
    popover.set_child(vbox);
    
    popover.signal_closed().connect([this]() {
        handler();
        if (secondary_handler) {
            secondary_handler();
        }
        is_popover_visible = false;
    });
    
    // Add check buttons
    for (const auto& opt : options) {
        auto check = Gtk::make_managed<Gtk::CheckButton>(opt);
        vbox.append(*check);
        check_buttons.push_back(check);
    }
}
    
void MultiSelectCombo::set_secondary_handler(std::function<void()> handler) {
    secondary_handler = handler;
}

MultiSelectCombo::~MultiSelectCombo() {
    popover.unparent();
}

std::vector<std::string> MultiSelectCombo::get_selected() const {
    std::vector<std::string> selected;
    for (auto check : check_buttons) {
        if (check->get_active())
            selected.push_back(check->get_label());
    }
    return selected;
}

// This function assumes that the order of labels and the order of check buttons here match
void MultiSelectCombo::set_active_mask(std::vector<std::string>& labels) {
    size_t label_idx = 0;

    for (auto& check: check_buttons) {
        if(label_idx < labels.size() && std::string(check->get_label()) == labels[label_idx]) {
            check->set_active(true);
            label_idx++;
        }
        else {
            check->set_active(false);
        }
    }
}

void MultiSelectCombo::on_button_clicked() {
    if (is_popover_visible) {
        throw std::runtime_error("on_button_clicked() called when popover is visible..");
    }
    else {
        popover.popup();
    }

    is_popover_visible = !is_popover_visible;
}

void MultiSelectCombo::update_list(const std::vector<std::string>& options) {
    // Remove all existing elements
    for (auto& check_button : check_buttons) {
        vbox.remove(*check_button);
    }

    check_buttons.clear();

    // Add the new set of elements
    for (auto& opt : options) {
        auto check_button = Gtk::make_managed<Gtk::CheckButton>(opt);
        vbox.append(*check_button);
        check_buttons.push_back(check_button);
    }
}

Slider::Slider(std::function<void()> handler) {
    auto adjustment = Gtk::Adjustment::create(0.0, 0.0, 1.0, 0.01, 0.1);

    set_adjustment(adjustment);
    set_value_pos(Gtk::PositionType::LEFT);
    set_digits(2);
    set_draw_value(true); 

    signal_value_changed().connect(handler);
}

