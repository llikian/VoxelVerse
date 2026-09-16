/***************************************************************************************************
 * @file  Application.hpp
 * @brief Declaration of the Application class
 **************************************************************************************************/

#pragma once

#include "assets/Camera.hpp"
#include "culling/Frustum.hpp"
#include "engine/Framebuffer.hpp"
#include "engine/SceneGraph.hpp"

/**
 * @class Application
 * @brief Core of the project. Assembles everything together and handles the main loop.
 */
class Application {
public:
    /**
     * @brief Constructor.
     */
    Application();

    /**
     * @brief Frees all resources.
     */
    ~Application();

    /**
     * @brief Executes the main loop of the application.
     */
    void run();

private:
    /**
     * @brief Draws the background, the scene and handles post processing.
     */
    void draw();

    /**
     * @brief Draws the imgui debug window.
     */
    void draw_imgui_windows();

    Camera camera; ///< The camera.

    SceneGraph scene_graph;  ///< Scene graph.
    Framebuffer framebuffer; ///< The framebuffer used to render.

    Frustum frustum; ///< The frustum used for culling.

    bool are_axes_drawn; ///< Whether the axes are drawn.

    vec3 sky_color_low;
    vec3 sky_color_high;
};
