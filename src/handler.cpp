#include <chrono>
#include <GL/glew.h>
#include "error.h"
#include "handler.h"
#include "ui.h"

constexpr float ROTATION_FACTOR = 1.0f;
float current_zoom = 1.0f;
constexpr float ZOOM_FACTOR = 0.01f;

namespace MVF {
    bool RenderHandler::initialized_glew = false;
    RenderHandler::RenderHandler(Renderer* renderer, bool is_spatial_handler) : renderer(renderer), is_spatial_renderer(is_spatial_handler) {
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

    bool RenderHandler::on_key_pressed(guint keyval, guint keycode, Gdk::ModifierType state) {
        if (state != Gdk::ModifierType::NO_MODIFIER_MASK || !is_spatial_renderer) {
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
}