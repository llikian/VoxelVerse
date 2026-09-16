/***************************************************************************************************
 * @file  Application.cpp
 * @brief Implementation of the Application class
 **************************************************************************************************/

#include "applications/Application.hpp"

#include "assets/AssetManager.hpp"
#include "engine/Context.hpp"
#include "glad/glad.h"
#include "maths/constants.hpp"
#include "mesh/primitives.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

Application::Application()
    : camera(vec3(0.0f, 10.0f, -5.0f), PI_HALF_F, 0.1f, 1024.0f),
      framebuffer(Context::window().get_width(), Context::window().get_height()),
      sky_color_low(0.0f, 0.105f, 0.191f),
      sky_color_high(0.123f, 0.285f, 0.583f) {
    /* ---- Event Handler ---- */
    EventHandler& event_handler = Context::event_handler();

    // event_handler.add_event_listener(WINDOW_SIZE_EVENT, [this]() { camera.update_projection_matrix(); });

    event_handler.bind_key(GLFW_KEY_W, true, [this] { camera.move_around(MovementDirection::FORWARD); });
    event_handler.bind_key(GLFW_KEY_A, true, [this] { camera.move_around(MovementDirection::LEFT); });
    event_handler.bind_key(GLFW_KEY_S, true, [this] { camera.move_around(MovementDirection::BACKWARD); });
    event_handler.bind_key(GLFW_KEY_D, true, [this] { camera.move_around(MovementDirection::RIGHT); });
    event_handler.bind_key(GLFW_KEY_SPACE, true, [this] { camera.move_around(MovementDirection::UPWARD); });
    event_handler.bind_key(GLFW_KEY_C, true, [this] { camera.move_around(MovementDirection::DOWNWARD); });

    event_handler.add_event_listener(CURSOR_POSITION_EVENT, [this] {
        EventHandler& event_handler = Context::event_handler();
        vec2 offset = event_handler.get_mouse_offset();
        if(!event_handler.is_cursor_visible()) { camera.look_around(offset.y, offset.x); }
    });

    /* ---- ImGui ---- */
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui::GetIO().IniFilename = "imgui.ini";
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplGlfw_InitForOpenGL(Context::window().get_glfw_window(), true);
    ImGui_ImplOpenGL3_Init();

    /* ---- Other ---- */
    // glfwSwapInterval(0); // disable vsync
    create_screen_mesh(mesh_screen);
}

Application::~Application() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Application::run() {
    //

    /* Main Loop */
    while(!Context::window().should_close()) {
        Context::event_handler().poll_and_handle_events();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

        draw_main_window();
        draw_imgui_windows();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        Context::window().swap_buffers();
    }
}

void Application::draw() {
    //
}

void Application::draw_main_window() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Main", nullptr);
    ImGui::PopStyleVar(1);

    ImVec2 avail = ImGui::GetContentRegionAvail();
    avail.x = avail.x < 1.0f ? 1.0f : avail.x;
    avail.y = avail.y < 1.0f ? 1.0f : avail.y;

    vec2 res = framebuffer.get_resolution();
    if(static_cast<int>(avail.x) != res.x || static_cast<int>(avail.y) != res.y) {
        framebuffer.resize(avail.x, avail.y);
        glViewport(0, 0, avail.x, avail.y);
        camera.update_projection_matrix(avail.x, avail.y);
    }

    draw_background();
    draw();

    ImGui::Image(framebuffer.get_texture_id(), avail, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

    ImGui::End();
}

void Application::draw_background() {
    framebuffer.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* ---- Background ---- */
    const Shader& background_shader = AssetManager::get_shader(SHADER_BACKGROUND);
    background_shader.use();
    background_shader.set_uniform("u_resolution", Context::window().get_resolution());
    background_shader.set_uniform("u_camera_direction", camera.get_direction());
    background_shader.set_uniform("u_camera_right", camera.get_right_vector());
    background_shader.set_uniform("u_camera_up", camera.get_up_vector());
    background_shader.set_uniform("u_sky_color_low", sky_color_low);
    background_shader.set_uniform("u_sky_color_high", sky_color_high);

    if(Context::event_handler().is_wireframe_enabled()) { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }
    mesh_screen.draw();
    if(Context::event_handler().is_wireframe_enabled()) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
}

void Application::draw_imgui_windows() {
    Framebuffer::bind_default();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui::Begin("Debug");

    vec3 camera_pos = camera.get_position();

    ImGui::Text("fps: %f f/s", 1.0f / Context::event_handler().get_delta());
    ImGui::Text("delta: %fs", Context::event_handler().get_delta());
    ImGui::Text("pos: ( %.3f ; %.3f ; %.3f )", camera_pos.x, camera_pos.y, camera_pos.z);

    ImGui::End();

    ImGui::Begin("Environment");
    ImGui::ColorEdit3("Low Sky Color", &sky_color_low.x);
    ImGui::ColorEdit3("High Sky Color", &sky_color_high.x);
    ImGui::End();

    ImGui::Begin("Camera");
    ImGui::Text("Camera:");
    ImGui::SliderFloat("Sensitivity", &camera.sensitivity, 0.05f, 1.0f);
    ImGui::SliderFloat("Movement Speed", &camera.movement_speed, 1.0f, 100.0f);
    ImGui::End();
}
