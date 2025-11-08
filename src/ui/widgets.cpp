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

HoverOverlay::HoverOverlay(Gtk::Widget& parent) {
    set_has_arrow(false);
    set_autohide(false); 
    set_parent(parent);
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
