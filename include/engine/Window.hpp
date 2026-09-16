/***************************************************************************************************
 * @file  Window.hpp
 * @brief Declaration of the Window class
 **************************************************************************************************/

#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "maths/vec2.hpp"

/**
 * @class Window
 * @brief Acts as a layer above the GLFW window.
 */
class Window {
public:
    /**
     * @brief Initializes GLFW and creates the window.
     */
    Window(const std::string& window_name, int _width, int _height);

    /**
     * @brief Destroys the GLFW window and terminates GLFW.
     */
    ~Window();

    /**
     * @return The GLFW window pointer.
     */
    GLFWwindow* get_glfw_window() const;

    /**
     * @return The width of the window.
     */
    int get_width() const;

    /**
     * @return The height of the window.
     */
    int get_height() const;

    /**
     * @brief Returns the window's aspect ratio.
     * @return The window's width divided by its height.
     */
    float get_aspect_ratio() const;

    /**
     * @brief Updates the size of the window.
     * @param width The window's current width.
     * @param height The window's current height.
     */
    void update_size(int width, int height);

    /**
     * @return A vec2 with the width of the window as the x component and its height as the y component.
     */
    vec2 get_resolution() const;

    /**
     * @return Whether the window should close.
     */
    bool should_close() const;

    /**
     * @brief Tells the glfw window it should close.
     */
    void set_should_close();

    /**
     * @brief Swaps the front and back buffers of the window.
     */
    void swap_buffers() const;

private:
    GLFWwindow* window; ///< The GLFW window pointer.
    int width;          ///< The width of the window.
    int height;         ///< The height of the window.
};

void glfw_error_callback(int code, const char* message);
