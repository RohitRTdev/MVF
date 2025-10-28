#include <chrono>
#include <GL/glew.h>
#include "error.h"
#include "handler.h"
#include "ui.h"

constexpr float ROTATION_FACTOR = 1.0f;
float current_zoom = 1.0f;
constexpr float ZOOM_FACTOR = 0.01f;

namespace MVF {
    RenderHandler::RenderHandler(MainWindow* parent) : parent(parent) {
        set_hexpand(true);
        set_vexpand(true);

        set_required_version(4, 6);
        set_has_depth_buffer(true);
        signal_realize().connect(sigc::mem_fun(*this, &RenderHandler::on_realize));
        signal_render().connect(sigc::mem_fun(*this, &RenderHandler::on_render), true);
        signal_resize().connect(sigc::mem_fun(*this, &RenderHandler::on_resize)); 
    }

    void RenderHandler::on_resize(int width, int height) {
#ifdef MVF_DEBUG
        std::cout << "GLArea resized to " << width << "x" << height << std::endl;
#endif
        make_current();
        parent->renderer.set_viewport(width, height);
    }

    void RenderHandler::on_realize() {
        Gtk::GLArea::on_realize();

        try {
            make_current();
            GLenum res = glewInit();
            if (res != GLEW_OK) {
                MVF::app_error(std::string("Error: ") + reinterpret_cast<const char*>(glewGetErrorString(res)));
            }
            parent->renderer.init(get_allocated_width(), get_allocated_height());
            
            // This is needed since gtk might realize and unrealize the context multiple times
            // We need to then recreate the buffers
            if (parent->loaded_scene) {
                parent->renderer.entity.resync();
            }
        } catch (const Gdk::GLError& gle) {
            MVF::app_error(std::string("Failed to realize GLArea ") + gle.what());
        }
    }

    bool RenderHandler::on_render(const Glib::RefPtr<Gdk::GLContext>& context) {
        parent->renderer.render();
        return true;
    }

    bool RenderHandler::on_key_pressed(guint keyval, guint keycode, Gdk::ModifierType state) {
        if (state != Gdk::ModifierType::NO_MODIFIER_MASK) {
            return false;
        }

        switch(keyval) { 
            case GDK_KEY_w: {
                parent->renderer.entity.rotate(ROTATION_FACTOR, 0, 0);
                break;
            }
            case GDK_KEY_s: {
                parent->renderer.entity.rotate(-ROTATION_FACTOR, 0, 0);
                break;

            }
            case GDK_KEY_a: {
                parent->renderer.entity.rotate(0, 0, ROTATION_FACTOR);
                break;
            }
            case GDK_KEY_d: {
                parent->renderer.entity.rotate(0, 0, -ROTATION_FACTOR);
                break;
            }
            case GDK_KEY_q: {
                parent->renderer.entity.rotate(0, ROTATION_FACTOR, 0);
                break;
            }
            case GDK_KEY_e: {
                parent->renderer.entity.rotate(0, -ROTATION_FACTOR, 0);
                break;
            }
            case GDK_KEY_z: {
                current_zoom += ZOOM_FACTOR;
                parent->renderer.entity.scale(current_zoom);
                break;
            }
            case GDK_KEY_x: {
                current_zoom -= ZOOM_FACTOR;
                parent->renderer.entity.scale(current_zoom);
                break;
            }
            default: return false;
        }

        queue_render();
        return true;
    } 
}