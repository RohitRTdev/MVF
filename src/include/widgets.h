#pragma once

#include <gtkmm.h>
#include <functional>

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

class MultiSelectCombo : public Gtk::Box {
public:
    MultiSelectCombo(const std::vector<std::string>& options, std::function<void()> on_click);
    ~MultiSelectCombo();
    std::vector<std::string> get_selected() const;
    void update_list(const std::vector<std::string>& options);
    void set_secondary_handler(std::function<void()> handler);
    void set_active_mask(std::vector<std::string>& labels);
private:
    Gtk::Button main_button;
    Gtk::Popover popover;
    Gtk::Box vbox;
    std::vector<Gtk::CheckButton*> check_buttons;
    std::function<void ()> handler, secondary_handler;
    bool is_popover_visible = false;
    
    void on_button_clicked();
};
