#include <chrono>
#include <format>
#include <epoxy/gl.h>
#include <ranges>
#include <limits>
#include <cmath>
#include "error.h"
#include "handler.h"
#include "ui.h"
#include "attrib.h"

constexpr float ROTATION_FACTOR = 1.0f;
constexpr float ZOOM_FACTOR = 0.01f;
constexpr float DETECT_THRESHOLD = 0.015f;
constexpr float RANGE_ZERO_WIDTH = 0.01f;

static std::pair<double, double> convert_to_ndc(double x, double y, size_t width, size_t height) {
    auto x_ndc = ((x / width) * 2.0) - 1;
    auto y_ndc = 1 - ((y / height) * 2.0);
    return std::make_pair(x_ndc, y_ndc);
}

namespace MVF {
    RenderHandler::RenderHandler(Renderer* renderer, bool has_depth_buffer) : renderer(renderer) {
        set_hexpand(true);
        set_vexpand(true);

        set_required_version(4, 6);
        set_has_depth_buffer(has_depth_buffer);
        
        int major, minor;
        get_required_version(major, minor);
        if (major != 4 && minor != 6) {
            MVF::app_error("GL version 4.6 core required");
        }
        
        signal_realize().connect(sigc::mem_fun(*this, &RenderHandler::on_realize));
        signal_unrealize().connect(sigc::mem_fun(*this, &RenderHandler::on_unrealize));
        signal_render().connect(sigc::mem_fun(*this, &RenderHandler::on_render), true);
        signal_resize().connect(sigc::mem_fun(*this, &RenderHandler::on_resize));
    }

    void RenderHandler::on_resize(int width, int height) {
        renderer->set_viewport(width, height);
        // propagate viewport size to attrib renderer for overlay coordinate mapping
        if (auto ar = dynamic_cast<AttribRenderer*>(renderer)) {
            ar->update_viewport_size(width, height);
        }
    }

    void RenderHandler::on_realize() {
        Gtk::GLArea::on_realize();

        try {
            make_current();
#ifdef MVF_DEBUG
            std::cout << "GL version: " << glGetString(GL_VERSION) << std::endl;
            std::cout << "Realizing GLArea..." << std::endl;
#endif
            renderer->init(get_allocated_width(), get_allocated_height()); 
            // This is needed since gtk might realize and unrealize the context multiple times
            // We need to then recreate the buffers
            renderer->resync();
        }
        catch (const Gdk::GLError& gle) {
            MVF::app_error(std::string("Failed to realize GLArea ") + gle.what());
        }
    }

    void RenderHandler::on_unrealize() {
#ifdef MVF_DEBUG
        std::cout << "Unrealizing GL context..." << std::endl;
#endif
        Gtk::GLArea::on_unrealize();
    }

    bool RenderHandler::on_render(const Glib::RefPtr<Gdk::GLContext>& context) {
        renderer->render();
        return true;
    }

    SpatialHandler::SpatialHandler(SpatialRenderer* renderer) : RenderHandler(renderer) {
        // Add GTK4 controllers for mouse
        auto click = Gtk::GestureClick::create();
        click->set_button(GDK_BUTTON_PRIMARY);
        click->signal_pressed().connect([this](int n_press, double x, double y) {
            // Always enable trackball on press
            trackball_active = true;
            last_x = static_cast<int>(x);
            last_y = static_cast<int>(y);
        });
        
        click->signal_released().connect([this](int n_press, double x, double y) {
            trackball_active = false;
        });
        
        add_controller(click);

        auto motion = Gtk::EventControllerMotion::create();
        motion->signal_motion().connect([this](double x, double y) {
            auto spatial = static_cast<SpatialRenderer*>(this->renderer);
            if (!trackball_active) return;
            // Trackball update
            int w = get_allocated_width();
            int h = get_allocated_height();
            auto map = [](int px, int py, int w, int h) {
                float nx = (2.0f * px - w) / (float)w;
                float ny = (h - 2.0f * py) / (float)h;
                float len2 = nx * nx + ny * ny;
                float nz = 0.0f;
                if (len2 <= 1.0f) nz = sqrtf(1.0f - len2);
                else {
                    float norm = 1.0f / sqrtf(len2);
                    nx *= norm; ny *= norm;
                }
                return Vector3f(nx, ny, nz);
            };
            Vector3f va = map(last_x, last_y, w, h);
            Vector3f vb = map((int)x, (int)y, w, h);
            Vector3f axis = va.cross(vb);
            float dot = va.x * vb.x + va.y * vb.y + va.z * vb.z;
            if (dot > 1.0f) dot = 1.0f;
            if (dot < -1.0f) dot = -1.0f;
            float angle = acosf(dot);
            if (axis.length() > 1e-5f && angle > 1e-5f) {
                axis.Normalize();
                Quaternion dq = Quaternion::FromAxisAngle(axis, angle);
                trackball_quat = dq * trackball_quat;
                trackball_quat.Normalize();
                // Apply to entity's world as a rotation matrix pre-multiplied
                Matrix4f R = QuaternionToMatrix(trackball_quat);
                spatial->set_world_orientation(R); // keep only trackball orientation
            }
            last_x = static_cast<int>(x);
            last_y = static_cast<int>(y);
            queue_render();
        });
        add_controller(motion);
    }

    AttribHandler::AttribHandler(AttribRenderer* renderer) : RenderHandler(renderer, false) {

        set_focusable(true);  // <-- REQUIRED
        auto mouse_click = Gtk::GestureClick::create();
        mouse_click->set_button(GDK_BUTTON_PRIMARY);
        mouse_click->signal_pressed().connect(sigc::mem_fun(*this, &AttribHandler::on_mouse_click));
        // Grab focus on mouse click to allow key events
        mouse_click->signal_pressed().connect([this](gint, double, double) {
            grab_focus();      // <-- focus obtained here
            });
        add_controller(mouse_click);
        // Add key controller for sampling toggle in parallel coordinates
        auto key_ctrl = Gtk::EventControllerKey::create();
        key_ctrl->signal_key_pressed().connect([this](guint keyval, guint keycode, Gdk::ModifierType state){
            if (state != static_cast<Gdk::ModifierType>(0)) return false;
            if (keyval == GDK_KEY_p) {
                auto ar = static_cast<AttribRenderer*>(this->renderer);
                ar->cycle_sample_period();
                this->queue_render();
                return true;
            }
            return false;
        }, false);
        add_controller(key_ctrl);
        auto motion = Gtk::EventControllerMotion::create();
        motion->signal_motion().connect([this](double x, double y) {
            auto [x_ndc, y_ndc] = convert_to_ndc(x, y, get_allocated_width(), get_allocated_height());
           
            auto x_bound = AXIS_LENGTH / 2;
            auto y_bound = field_comps.size() == 1 ? DETECT_THRESHOLD :  AXIS_LENGTH / 2;
            
            if (field_comps.empty() || std::abs(y_ndc) > y_bound || std::abs(x_ndc) > x_bound) {
                this->mouse_overlay.hide_now();
                return;
            }

            // Parallel coordinates (>=3 components): show only when close to an axis
            if (field_comps.size() > 2) {
                float span = AXIS_LENGTH;
                size_t n = field_comps.size();
                float dx = span / (n - 1);
                float x0 = -AXIS_LENGTH / 2;
                int nearest = -1;
                float best_dist = 1.0f; // ndc threshold
                for (size_t i = 0; i < n; ++i) {
                    float axis_x = x0 + i * dx;
                    float d = std::abs(x_ndc - axis_x);
                    if (d < best_dist) { best_dist = d; nearest = static_cast<int>(i);}    
                }
                constexpr float AXIS_DETECT_THRESHOLD = 0.02f; // tighten axis hover sensitivity
                if (nearest == -1 || best_dist > AXIS_DETECT_THRESHOLD) {
                    this->mouse_overlay.hide_now();
                    return;
                }
                // Map vertical coordinate to field value
                try {
                    auto val_f = static_cast<AttribRenderer*>(this->renderer)->get_field_point(y_ndc, nearest);
                    std::string text = std::format("{}={:.2f}", field_comps[nearest], val_f);
                    this->mouse_overlay.show_at(static_cast<int>(x), static_cast<int>(y), text);
                } catch(...) {
                    this->mouse_overlay.hide_now();
                }
                // live update selection during range drag if active (no highlight here)
                if (activate_range_selection && !is_point_trait) {
                    auto ar = static_cast<AttribRenderer*>(this->renderer);
                    float wy = y_ndc * (AXIS_LENGTH/2.0f);
                    size_t nAx = n;
                    if (parallel_range_selection.size() != nAx) parallel_range_selection.assign(nAx, {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()});
                    if (range_has_first.size() != nAx) range_has_first.assign(nAx, false);

                    // rebuild all pending crosses: both first and finalized second for each axis
                    ar->clear_parallel_pending_markers();
                    for (size_t i = 0; i < nAx; ++i) {
                        if (range_has_first[i]) {
                            ar->add_parallel_pending_marker(i, parallel_range_selection[i].first);
                            if (!std::isnan(parallel_range_selection[i].second) && parallel_range_selection[i].second != parallel_range_selection[i].first) {
                                ar->add_parallel_pending_marker(i, parallel_range_selection[i].second);
                            }
                        }
                    }
                    // for current axis, if only first is set and second not yet finalized, show live second
                    if (range_has_first[nearest] && (std::isnan(parallel_range_selection[nearest].second) || parallel_range_selection[nearest].second == parallel_range_selection[nearest].first)) {
                        ar->add_parallel_pending_marker(nearest, wy);
                    }
                    this->queue_render();
                }
                return;
            }

            // 1 or 2 components (existing behavior)
            auto x_f = static_cast<AttribRenderer*>(this->renderer)->get_field_point(x_ndc, 0);
            std::string text = std::format("{}={:.2f}", field_comps[0], x_f);
            if (field_comps.size() == 2) {
                auto y_f = static_cast<AttribRenderer*>(this->renderer)->get_field_point(y_ndc, 1);
                text.append(std::format("\n{}={:.2f}", field_comps[1], y_f));
            }
            this->mouse_overlay.show_at(static_cast<int>(x), static_cast<int>(y), text);
            
            if (activate_range_selection && field_comps.size() <= 2) {
                field_comps.size() == 1 ? static_cast<AttribRenderer*>(this->renderer)->modify_range_trait(x_ndc) :
                static_cast<AttribRenderer*>(this->renderer)->modify_range_trait(x_ndc, y_ndc);
            }
        });
        
        mouse_click->signal_released().connect([this] (int, double, double) {
            activate_range_selection = false;
        });

        motion->signal_leave().connect([this] {
            mouse_overlay.hide_now();
            activate_range_selection = false;
        });

        add_controller(motion);
   
        signal_realize().connect([this] {
            mouse_overlay.set_parent(*this);
        });
        
        signal_unrealize().connect([this] {
            mouse_overlay.unparent();
            });
    }

    void AttribHandler::set_field_info(std::vector<AxisDesc>& descriptors) {
        field_comps = descriptors | std::views::transform([](const AxisDesc& d){ return d.comp_name; }) 
        | std::ranges::to<std::vector>();
        
        static_cast<AttribRenderer*>(renderer)->set_attrib_space_axis(descriptors);
        // reset parallel selection state
        parallel_point_selection.clear();
        parallel_range_selection.clear();
        range_has_first.clear();
        if (field_comps.size() > 2) {
            parallel_point_selection.resize(field_comps.size(), std::numeric_limits<float>::quiet_NaN());
            parallel_range_selection.resize(field_comps.size(), {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()});
            range_has_first.resize(field_comps.size(), false);
        }
    }
    
    void AttribHandler::clear_field_info() {
        field_comps.clear();
        parallel_point_selection.clear();
        parallel_range_selection.clear();
        range_has_first.clear();
    }

    bool SpatialHandler::on_key_pressed(guint keyval, guint keycode, Gdk::ModifierType state) {
        if (static_cast<int>(state) != 0) {
            return false;
        }

        auto spatial_renderer = static_cast<SpatialRenderer*>(renderer);

        switch (keyval) {
        case GDK_KEY_z: {
            current_zoom += ZOOM_FACTOR;
            spatial_renderer->entity.scale(current_zoom);
            break;
        }
        case GDK_KEY_x: {
            current_zoom -= ZOOM_FACTOR;
            spatial_renderer->entity.scale(current_zoom);
            break;
        }
        case GDK_KEY_Up: {
            if (spatial_renderer->entity.get_mode() == EntityMode::SCALAR_SLICE) {
                float t = spatial_renderer->entity.get_slice_position();
                spatial_renderer->entity.set_slice_position(std::min(1.0f, t + 0.01f));
            } else return false;
            break;
        }
        case GDK_KEY_Down: {
            if (spatial_renderer->entity.get_mode() == EntityMode::SCALAR_SLICE) {
                float t = spatial_renderer->entity.get_slice_position();
                spatial_renderer->entity.set_slice_position(std::max(0.0f, t - 0.01f));
            } else return false;
            break;
        }
        case GDK_KEY_Left: {
            if (spatial_renderer->entity.get_mode() == EntityMode::SCALAR_SLICE) {
                int axis = spatial_renderer->entity.get_slice_axis();
                if (axis >= 0) {
                    axis = (axis + 2) % 3; // cycle backwards
                    spatial_renderer->entity.set_slice_axis(axis);
                }
            } else return false;
            break;
        }
        case GDK_KEY_Right: {
            if (spatial_renderer->entity.get_mode() == EntityMode::SCALAR_SLICE) {
                int axis = spatial_renderer->entity.get_slice_axis();
                if (axis >= 0) {
                    axis = (axis + 1) % 3; // cycle forward
                    spatial_renderer->entity.set_slice_axis(axis);
                }
            } else return false;
            break;
        }
        default: return false;
        }

        queue_render();
        return true;
    } 
        
    void SpatialHandler::reset_camera() {
        current_zoom = 1.0f;
        static_cast<SpatialRenderer*>(renderer)->entity.reset_transform();
        trackball_quat = Quaternion();    
        queue_render();
    }

    void AttribHandler::on_mouse_click(int n_press, double x, double y) {
        auto width = get_allocated_width();
        auto height = get_allocated_height();
        auto [x_ndc, y_ndc] = convert_to_ndc(x, y, width, height);

        if (field_comps.size() > 2) {
            float span = AXIS_LENGTH; size_t n = field_comps.size(); float dx = span / (n - 1); float x0 = -AXIS_LENGTH / 2;
            int nearest = -1; float best_dist = 1.0f;
            for (size_t i = 0; i < n; ++i) { float axis_x = x0 + i * dx; float d = std::abs(x_ndc - axis_x); if (d < best_dist) { best_dist = d; nearest = (int)i; } }
            constexpr float AXIS_DETECT_THRESHOLD = 0.02f; if (nearest == -1 || best_dist > AXIS_DETECT_THRESHOLD) return;
            make_current(); auto ar = static_cast<AttribRenderer*>(renderer); float wy = y_ndc * (AXIS_LENGTH/2.0f);
            if (is_point_trait) {
                if (parallel_point_selection.size() != n) parallel_point_selection.assign(n, std::numeric_limits<float>::quiet_NaN());
                if (!std::isnan(parallel_point_selection[nearest])) return; // one point per axis per line build
                parallel_point_selection[nearest] = wy; ar->add_parallel_pending_marker(nearest, wy);
                bool all_set = true; for (size_t i = 0; i < n; ++i) if (std::isnan(parallel_point_selection[i])) { all_set = false; break; }
                if (all_set) {
                    ar->select_parallel_line_by_points(parallel_point_selection);
                    std::fill(parallel_point_selection.begin(), parallel_point_selection.end(), std::numeric_limits<float>::quiet_NaN());
                    handle_traits = true; // flag for UI apply enable
                }
            } else { // range selection
                if (parallel_range_selection.size() != n) parallel_range_selection.assign(n, {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()});
                if (range_has_first.size() != n) range_has_first.assign(n, false);
                if (!range_has_first[nearest]) {
                    parallel_range_selection[nearest].first = wy; parallel_range_selection[nearest].second = wy; range_has_first[nearest] = true; activate_range_selection = true; ar->add_parallel_pending_marker(nearest, wy);
                } else if (std::isnan(parallel_range_selection[nearest].second) || parallel_range_selection[nearest].second == parallel_range_selection[nearest].first) {
                    parallel_range_selection[nearest].second = wy; ar->add_parallel_pending_marker(nearest, wy); // second cross
                } else { // already two points this axis -> ignore extra
                    return;
                }
                bool all_axes_have_two = true;
                for (size_t i = 0; i < n; ++i) {
                    if (!range_has_first[i] || std::isnan(parallel_range_selection[i].second) || parallel_range_selection[i].second == parallel_range_selection[i].first) { all_axes_have_two = false; break; }
                }
                if (all_axes_have_two) {
                    ar->select_parallel_region_by_ranges(parallel_range_selection);
                    activate_range_selection = false;
                    std::fill(range_has_first.begin(), range_has_first.end(), false);
                    handle_traits = true;
                }
            }
            queue_render(); return;
        }

        if (field_comps.size() != 1 && field_comps.size() != 2) return;
        auto x_bound = AXIS_LENGTH / 2; auto y_bound = field_comps.size() == 1 ? DETECT_THRESHOLD : AXIS_LENGTH / 2;
        if (std::abs(y_ndc) > y_bound || std::abs(x_ndc) > x_bound) return;
        handle_traits = true; make_current();
        if (is_point_trait) {
            field_comps.size() == 1 ? static_cast<AttribRenderer*>(renderer)->set_point_trait(x_ndc) : static_cast<AttribRenderer*>(renderer)->set_point_trait(x_ndc, y_ndc);
        } else {
            activate_range_selection = true;
            field_comps.size() == 1 ? static_cast<AttribRenderer*>(renderer)->set_range_trait(x_ndc, x_ndc + RANGE_ZERO_WIDTH) : static_cast<AttribRenderer*>(renderer)->set_range_trait(x_ndc, y_ndc, RANGE_ZERO_WIDTH, RANGE_ZERO_WIDTH);
        }
        queue_render();
    }
        
    void AttribHandler::set_point_trait_mode(bool set_point_trait) {
        is_point_trait = set_point_trait;
        // reset parallel selection state when mode changes
        std::fill(range_has_first.begin(), range_has_first.end(), false);
        auto ar = static_cast<AttribRenderer*>(renderer);
        this->make_current();
        ar->clear_parallel_pending_markers();
    }
}