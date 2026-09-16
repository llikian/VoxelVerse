/***************************************************************************************************
 * @file  Context.hpp
 * @brief Declaration of the Context class
 **************************************************************************************************/

#pragma once

#include "glad/glad.h"

#include "engine/EventHandler.hpp"
#include "engine/Window.hpp"
#include "imgui.h"

/**
 * @class Context
 * @brief
 */
class Context {
public:
    Context(const Context&) = delete;            ///< Delete copy constructor.
    Context& operator=(const Context&) = delete; ///< Deleted copy operator.

    static void init();

    static Context* context() { return _context; }

    static Window& window() { return _context->_window; }

    static EventHandler& event_handler() { return _context->_event_handler; }

    bool is_main_window_hovered;
    vec2 mouse_pos_in_main_window;
    vec2 framebuffer_resolution;

private:
    Context();
    ~Context() = default;

    inline static Context* _context { nullptr };
    Window _window;
    EventHandler _event_handler;
};

void opengl_error_callback(unsigned int source,
                           unsigned int type,
                           unsigned int id,
                           unsigned int severity,
                           int /* length */,
                           const GLchar* message,
                           const void* /* userParam */);
