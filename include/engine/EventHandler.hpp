/***************************************************************************************************
 * @file  EventHandler.hpp
 * @brief Declaration of the EventHandler class
 **************************************************************************************************/

#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <functional>
#include <unordered_map>
#include "engine/Window.hpp"
#include "maths/vec2.hpp"

enum EventType : unsigned char {
    WINDOW_SIZE_EVENT,
    // FRAMEBUFFER_SIZE_EVENT,
    // KEY_PRESS_EVENT,
    // KEY_RELEASE_EVENT,
    CURSOR_POSITION_EVENT,
    // MOUSE_BUTTON_EVENT,

    EVENT_TYPE_COUNT
};

class EventHandler {
public:
    using Callback = std::function<void()>;

    explicit EventHandler(Window& win);

    void bind_key(int key, bool repeatable, const Callback& action);
    void unbind_key(int key);

    void poll_and_handle_events();

    template <typename Func>
    void set_left_click_func(Func&& func) {
        left_click_callback = std::forward<Func>(func);
    }

    /* ---- Getters ---- */

    vec2 get_mouse_position() { return mouse_position; }

    vec2 get_mouse_offset() { return mouse_offset; }

    float get_time() { return time; }

    float get_delta() { return delta; }

    bool is_cursor_visible() { return b_is_cursor_visible; }

    bool is_face_culling_enabled() { return b_is_face_culling_enabled; }

    bool is_wireframe_enabled() { return b_is_wireframe_enabled; }

    /* ---- Toggles ---- */
    void toggle_cursor_visible();
    void toggle_face_culling();
    void toggle_wireframe();

    void add_event_listener(EventType event_type, Callback&& callback) { callbacks[event_type].push_back(callback); }

private:
    void handle_window_size_event(int width, int height);
    void handle_framebuffer_size_event(int width, int height);
    void handle_key_press_event(int key);
    void handle_key_release_event(int key);
    void handle_cursor_position_event(int position_x, int position_y);
    void handle_mouse_button_event(int button, int action);

    Window& window;

    std::unordered_map<int, Callback> key_bindings; ///< Stores the function associated with each key.
    std::unordered_map<int, bool> repeatable_keys;  ///< Stores repeatable keys and whether they are active.

    std::array<std::vector<Callback>, EVENT_TYPE_COUNT> callbacks;

    Callback left_click_callback; ///< The function executed when left click is pressed.

    vec2 mouse_position; ///< The position of the cursor of the mouse on the window.
    vec2 mouse_offset;   ///< The offset between the current position of the cursor and where it was before

    float time;  ///< How much time elapsed since the beginning of the program.
    float delta; ///< How much time passed since the last frame.

    bool b_is_cursor_visible;       ///< Whether the mouse's cursor is visible.
    bool b_is_face_culling_enabled; ///< Whether face culling is enabled.
    bool b_is_wireframe_enabled;    ///< Whether wireframe mode is enabled.

    /* ---- Friend Functions ---- */
    friend void window_size_callback(GLFWwindow* window, int width, int height);
    friend void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    friend void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    friend void cursor_position_callback(GLFWwindow* window, double position_x, double position_y);
    friend void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
};
