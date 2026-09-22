/***************************************************************************************************
 * @file  Application.hpp
 * @brief Declaration of the Application class
 **************************************************************************************************/

#pragma once

#include "assets/Camera.hpp"
#include "engine/Framebuffer.hpp"
#include "mesh/Mesh.hpp"

using Block = std::uint16_t; // All 16 bits represent the block_id
constexpr std::uint8_t BLOCK_BIT_COUNT = sizeof(Block) * 8;

struct Chunk {
    Block blocks[16][16][16];
};

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
    void draw();
    void draw_main_window();

    /**
     * @brief Draws the background.
     */
    void draw_background();

    /**
     * @brief Draws the imgui debug window.
     */
    void draw_imgui_windows();

    Camera camera; ///< The camera.

    Framebuffer framebuffer; ///< The framebuffer used to render.

    vec3 sky_color_low;
    vec3 sky_color_high;

    Mesh mesh_screen;
};
