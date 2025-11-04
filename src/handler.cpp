#include <chrono>
#include <format>
#include <GL/glew.h>
#include "error.h"
#include "handler.h"
#include "ui.h"
#include "attrib.h"

constexpr float ROTATION_FACTOR = 1.0f;
float current_zoom = 1.0f;
constexpr float ZOOM_FACTOR = 0.01f;
constexpr float DETECT_THRESHOLD = 0.015f;

// Widget -> Top left is (0, 0) and bottom right is (max_width, max_height)
// NDC -> Center is (0, 0). Y increases upwards and X increases towards the right
static std::pair<double, double> convert_to_ndc(double x, double y, size_t width, size_t height) {
    auto x_ndc = ((x / width) * 2.0) - 1;
    auto y_ndc = 1 - ((y / height) * 2.0);

    return std::make_pair(x_ndc, y_ndc);
}


namespace MVF {
    bool RenderHandler::initialized_glew = false;
    RenderHandler::RenderHandler(Renderer* renderer) : renderer(renderer) {
        set_hexpand(true);
        set_vexpand(true);

        set_required_version(4, 6);
        set_has_depth_buffer(true);
        signal_realize().connect(sigc::mem_fun(*this, &RenderHandler::on_realize));
        signal_unrealize().connect(sigc::mem_fun(*this, &RenderHandler::on_unrealize));
        signal_render().connect(sigc::mem_fun(*this, &RenderHandler::on_render), true);
        signal_resize().connect(sigc::mem_fun(*this, &RenderHandler::on_resize)); 
    }

    void RenderHandler::on_resize(int width, int height) {
        renderer->set_viewport(width, height);
    }

    void RenderHandler::on_realize() {
        Gtk::GLArea::on_realize();

        try {
            make_current();
            if (!initialized_glew) {
                GLenum res = glewInit();
                if (res != GLEW_OK) {
                    MVF::app_error(std::string("Error: ") + reinterpret_cast<const char*>(glewGetErrorString(res)));
                }

                initialized_glew = true;
            }
            
            std::cout << "Realizing GLArea..." << std::endl;
            renderer->init(get_allocated_width(), get_allocated_height()); 
            // This is needed since gtk might realize and unrealize the context multiple times
            // We need to then recreate the buffers
            renderer->resync();
        } catch (const Gdk::GLError& gle) {
            MVF::app_error(std::string("Failed to realize GLArea ") + gle.what());
        }
    }
    
    void RenderHandler::on_unrealize() {
#ifdef MVF_DEBUG
        std::cout << "Unrealizing GL context..." << std::endl;
#endif
        Gtk::GLArea::on_unrealize();
        
        renderer->unload();
    }

    bool RenderHandler::on_render(const Glib::RefPtr<Gdk::GLContext>& context) {
        renderer->render();
        return true;
    }
        
    SpatialHandler::SpatialHandler(SpatialRenderer* renderer) : RenderHandler(renderer)
    {}

    AttribHandler::AttribHandler(AttribRenderer* renderer) : RenderHandler(renderer), mouse_overlay(*this) {
        auto mouse_click = Gtk::GestureClick::create();
        mouse_click->set_button(GDK_BUTTON_PRIMARY);
        mouse_click->signal_pressed().connect(sigc::mem_fun(*this, &AttribHandler::on_mouse_click));
        add_controller(mouse_click);
        auto motion = Gtk::EventControllerMotion::create();
        motion->signal_motion().connect([this] (double x, double y) {
            auto [x_ndc, y_ndc] = convert_to_ndc(x, y, get_allocated_width(), get_allocated_height());
            
            if (num_dimensions == 0 || std::abs(y_ndc) > DETECT_THRESHOLD || std::abs(x_ndc) > AXIS_LENGTH / 2) {
                this->mouse_overlay.hide_now();
                return;
            }

            auto [x_f, _] = static_cast<AttribRenderer*>(this->renderer)->get_field_point(x_ndc, y_ndc, 0);
            this->mouse_overlay.show_at(static_cast<int>(x), static_cast<int>(y), std::format("U={:.2f}", x_f));
        });
        add_controller(motion);
    
        signal_unrealize().connect([this] {
            mouse_overlay.unparent();
        });
    }
        
    void AttribHandler::set_field_info(std::vector<AxisDesc>& descriptors) {
        num_dimensions = descriptors.size();
        static_cast<AttribRenderer*>(renderer)->set_attrib_space_axis(descriptors);
    }

    bool SpatialHandler::on_key_pressed(guint keyval, guint keycode, Gdk::ModifierType state) {
        if (state != Gdk::ModifierType::NO_MODIFIER_MASK) {
            return false;
        }

        auto spatial_renderer = static_cast<SpatialRenderer*>(renderer);

        switch(keyval) { 
            case GDK_KEY_w: {
                spatial_renderer->entity.rotate(ROTATION_FACTOR, 0, 0);
                break;
            }
            case GDK_KEY_s: {
                spatial_renderer->entity.rotate(-ROTATION_FACTOR, 0, 0);
                break;

            }
            case GDK_KEY_a: {
                spatial_renderer->entity.rotate(0, 0, ROTATION_FACTOR);
                break;
            }
            case GDK_KEY_d: {
                spatial_renderer->entity.rotate(0, 0, -ROTATION_FACTOR);
                break;
            }
            case GDK_KEY_q: {
                spatial_renderer->entity.rotate(0, ROTATION_FACTOR, 0);
                break;
            }
            case GDK_KEY_e: {
                spatial_renderer->entity.rotate(0, -ROTATION_FACTOR, 0);
                break;
            }
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
            default: return false;
        }

        queue_render();
        return true;
    } 

    void AttribHandler::on_mouse_click(int n_press, double x, double y) {
        auto width = get_allocated_width();
        auto height = get_allocated_height();

        auto [x_ndc, y_ndc] = convert_to_ndc(x, y, width, height);

        if (std::abs(y_ndc) > DETECT_THRESHOLD) {
            return;
        }

        make_current();
        static_cast<AttribRenderer*>(renderer)->set_point_trait(x_ndc);
        queue_render();
    }
}