/***************************************************************************************************
 * @file  Window.cpp
 * @brief Implementation of the Window class
 **************************************************************************************************/

#include "engine/Window.hpp"

Window::Window(const std::string& window_name, int _width, int _height) : window(nullptr) {
    if(!glfwInit()) { throw std::runtime_error("Failed to initialize GLFW."); }
    glfwSetErrorCallback(glfw_error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(_width, _height, window_name.c_str(), nullptr, nullptr);
    if(window == nullptr) { throw std::runtime_error("Failed to create window."); }

    glfwMakeContextCurrent(window);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwMaximizeWindow(window);
    glfwGetWindowSize(window, &width, &height);
}

Window::~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

GLFWwindow* Window::get_glfw_window() const {
    return window;
}

int Window::get_width() const {
    return width;
}

int Window::get_height() const {
    return height;
}

float Window::get_aspect_ratio() const {
    return static_cast<float>(width) / static_cast<float>(height);
}

void Window::update_size(int width, int height) {
    this->width = width;
    this->height = height;
}

vec2 Window::get_resolution() const {
    return vec2(width, height);
}

bool Window::should_close() const {
    return glfwWindowShouldClose(get_glfw_window());
}

void Window::set_should_close() {
    glfwSetWindowShouldClose(window, true);
}

void Window::swap_buffers() const {
    glfwSwapBuffers(get_glfw_window());
}

void glfw_error_callback(int code, const char* message) {
    std::cerr << "GLFW Error '" << code << "' : " << message << '\n';
}
