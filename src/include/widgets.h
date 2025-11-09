#pragma once

#include <gtkmm.h>

class OverlayProgressBar : public Gtk::Box {
public:
    OverlayProgressBar();
    void show();
    void hide();
    void set_fraction(double fraction);
private:
    Gtk::ProgressBar progress_bar;
    Gtk::Label label_progress;
};

class HoverOverlay : public Gtk::Popover {
public:
    HoverOverlay();
    void show_at(int x, int y, const std::string& text);
    void hide_now();

private:
    Gtk::Label label;
};
