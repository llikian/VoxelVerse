/***************************************************************************************************
 * @file  EventHandler.cpp
 * @brief Implementation of the EventHandler class
 **************************************************************************************************/

#include "engine/EventHandler.hpp"

#include "engine/callbacks.hpp"
#include "glad/glad.h"

EventHandler::EventHandler(Window& win)
    : window(win),
      left_click_callback(nullptr),
      time(glfwGetTime()),
      delta(0.0f),
      b_is_cursor_visible(glfwGetInputMode(window.get_glfw_window(), GLFW_CURSOR) == GLFW_CURSOR_NORMAL),
      b_is_face_culling_enabled(true),
      b_is_wireframe_enabled(false) {

    /* ---- Callbacks ---- */
    glfwSetWindowSizeCallback(window.get_glfw_window(), window_size_callback);
    glfwSetFramebufferSizeCallback(window.get_glfw_window(), framebuffer_size_callback);
    glfwSetKeyCallback(window.get_glfw_window(), key_callback);
    glfwSetCursorPosCallback(window.get_glfw_window(), cursor_position_callback);
    glfwSetMouseButtonCallback(window.get_glfw_window(), mouse_button_callback);

    /* ---- Key Bindings ---- */
    bind_key(GLFW_KEY_ESCAPE, false, [win_ptr = &window] { win_ptr->set_should_close(); });

    bind_key(GLFW_KEY_TAB, false, [this] { toggle_cursor_visible(); });
    bind_key(GLFW_KEY_F, false, [this] { toggle_face_culling(); });
    bind_key(GLFW_KEY_Z, false, [this] { toggle_wireframe(); });
}

void EventHandler::bind_key(int key, bool repeatable, const Callback& action) {
    key_bindings.emplace(key, action);
    if(repeatable) { repeatable_keys.emplace(key, false); }
}

void EventHandler::unbind_key(int key) {
    key_bindings.erase(key);
    repeatable_keys.erase(key);
}

void EventHandler::poll_and_handle_events() {
    glfwPollEvents();

    float temp_time = glfwGetTime();
    delta = temp_time - time;
    time = temp_time;

    /* Repeatable Keys */
    for(const auto& [key, is_active] : repeatable_keys) {
        if(is_active) { key_bindings[key](); }
    }
}

void EventHandler::handle_window_size_event(int width, int height) {
    window.update_size(width, height);

    for(const auto& callback : callbacks[WINDOW_SIZE_EVENT]) { callback(); }
}

void EventHandler::handle_framebuffer_size_event(int /* width */, int /* height */) {
    // glViewport(0, 0, width, height);
}

void EventHandler::handle_key_press_event(int key) {
    auto repeatable_key_iterator = repeatable_keys.find(key);

    if(repeatable_key_iterator != repeatable_keys.end()) {
        repeatable_key_iterator->second = true;
    } else {
        auto action_iterator = key_bindings.find(key);
        if(action_iterator != key_bindings.end()) { action_iterator->second(); }
    }
}

void EventHandler::handle_key_release_event(int key) {
    auto repeatable_key_iterator = repeatable_keys.find(key);
    if(repeatable_key_iterator != repeatable_keys.end()) { repeatable_key_iterator->second = false; }
}

void EventHandler::handle_cursor_position_event(int position_x, int position_y) {
    mouse_offset.x = position_x - mouse_position.x;
    mouse_offset.y = position_y - mouse_position.y;

    mouse_position.x = position_x;
    mouse_position.y = position_y;

    for(const auto& callback : callbacks[CURSOR_POSITION_EVENT]) { callback(); }
}

void EventHandler::handle_mouse_button_event(int button, int action) {
    if(action == GLFW_PRESS) {
        if(button == GLFW_MOUSE_BUTTON_LEFT) {
            if(left_click_callback != nullptr) { left_click_callback(); }
        }
    }
}

void EventHandler::toggle_cursor_visible() {
    b_is_cursor_visible = !b_is_cursor_visible;

    if(b_is_cursor_visible) {
        glfwSetInputMode(window.get_glfw_window(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
        glfwSetInputMode(window.get_glfw_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void EventHandler::toggle_face_culling() {
    (b_is_face_culling_enabled ? glDisable : glEnable)(GL_CULL_FACE);
    b_is_face_culling_enabled = !b_is_face_culling_enabled;
}

void EventHandler::toggle_wireframe() {
    glPolygonMode(GL_FRONT_AND_BACK, b_is_wireframe_enabled ? GL_FILL : GL_LINE);
    b_is_wireframe_enabled = !b_is_wireframe_enabled;
}
