/***************************************************************************************************
 * @file  callbacks.cpp
 * @brief Implementations of callback functions.
 **************************************************************************************************/

#include "engine/callbacks.hpp"

#include "engine/Context.hpp"

void window_size_callback(GLFWwindow* /* window */, int width, int height) {
    Context::event_handler().handle_window_size_event(width, height);
}

void framebuffer_size_callback(GLFWwindow* /* window */, int width, int height) {
    Context::event_handler().handle_framebuffer_size_event(width, height);
}

void key_callback(GLFWwindow* /* window */, int key, int /* scancode */, int action, int /* mods */) {
    if(action == GLFW_PRESS) {
        Context::event_handler().handle_key_press_event(key);
    } else if(action == GLFW_RELEASE) {
        Context::event_handler().handle_key_release_event(key);
    }
}

void cursor_position_callback(GLFWwindow* /* window */, double position_x, double position_y) {
    Context::event_handler().handle_cursor_position_event(position_x, position_y);
}

void mouse_button_callback(GLFWwindow* /* window */, int button, int action, int /* mods */) {
    Context::event_handler().handle_mouse_button_event(button, action);
}
