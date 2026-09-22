/***************************************************************************************************
 * @file  Application.cpp
 * @brief Implementation of the Application class
 **************************************************************************************************/

#include "applications/Application.hpp"

#include "assets/AssetManager.hpp"
#include "engine/Context.hpp"
#include "glad/glad.h"
#include "maths/constants.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

Application::Application()
    : camera(vec3(0.0f, 10.0f, -5.0f), PI_HALF_F, 0.1f, 1024.0f),
      scene_graph(camera),
      framebuffer(Context::window().get_width(), Context::window().get_height()),
      are_axes_drawn(false),
      sky_color_low(0.0f, 0.105f, 0.191f),
      sky_color_high(0.123f, 0.285f, 0.583f) {
    /* ---- Event Handler ---- */
    EventHandler& event_handler = Context::event_handler();
    event_handler.bind_key(GLFW_KEY_Q, false, [this] { are_axes_drawn = !are_axes_drawn; });

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
    ImGui::GetIO().IniFilename = "data/imgui.ini";
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplGlfw_InitForOpenGL(Context::window().get_glfw_window(), true);
    ImGui_ImplOpenGL3_Init();

    /* ---- Other ---- */
    // glfwSwapInterval(0); // disable vsync
}

Application::~Application() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Application::run() {
    // scene_graph.add_gltf_scene_node("Buggy", 0, "data/models/buggy.glb");

    unsigned int sponza = scene_graph.add_gltf_scene_node("Sponza", 0, "data/models/sponza/Sponza.gltf");
    scene_graph.transforms[sponza].set_local_scale(10.0f);

    /* Main Loop */
    while(!Context::window().should_close()) {
        Context::event_handler().poll_and_handle_events();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

        frustum.update(camera);

        // draw();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoTitleBar);
        ImGui::PopStyleVar(1);

        ImVec2 avail = ImGui::GetContentRegionAvail();
        avail.x = avail.x < 1.0f ? 1.0f : avail.x;
        avail.y = avail.y < 1.0f ? 1.0f : avail.y;

        ImVec2 mouse_pos = ImGui::GetMousePos();
        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 window_content_region = ImGui::GetWindowContentRegionMin();
        Context::context()->is_main_window_hovered = ImGui::IsWindowHovered();
        Context::context()->mouse_pos_in_main_window = vec2(mouse_pos.x - window_pos.x - window_content_region.x,
                                                            mouse_pos.y - window_pos.y - window_content_region.y);

        vec2 res = framebuffer.get_resolution();
        if(static_cast<int>(avail.x) != res.x || static_cast<int>(avail.y) != res.y) {
            framebuffer.resize(avail.x, avail.y);
            glViewport(0, 0, avail.x, avail.y);
            camera.update_projection_matrix(avail.x, avail.y);
            Context::context()->framebuffer_resolution = vec2(avail.x, avail.y);
        }

        draw();

        ImGui::Image(framebuffer.get_texture_id(), avail, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

        ImGui::End();

        draw_imgui_windows();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        Context::window().swap_buffers();
    }
}

void Application::draw() {
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
    AssetManager::get_mesh("screen").draw();
    if(Context::event_handler().is_wireframe_enabled()) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }

    /* ---- Scene ---- */
    scene_graph.draw(frustum);

    // /* ---- Post Processing ---- */
    // const Shader& post_processing_shader = AssetManager::get_shader(SHADER_POST_PROCESSING);
    // post_processing_shader.use();
    // post_processing_shader.set_uniform("u_texture", 0);
    // post_processing_shader.set_uniform("u_texture_resolution", framebuffer.get_resolution());
    // post_processing_shader.set_uniform_if_exists("u_resolution", Context::window().get_resolution());
    // framebuffer.bind_texture(0);

    Framebuffer::bind_default();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // if(Context::event_handler().is_wireframe_enabled()) { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }
    // AssetManager::get_mesh("screen").draw();
    // if(Context::event_handler().is_wireframe_enabled()) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
}

ImVec2 operator-(const ImVec2& left, const ImVec2& right) {
    return ImVec2(left.x - right.x, left.y - right.y);
}

void Application::draw_imgui_windows() {
    ImGui::Begin("Debug");

    vec3 camera_pos = camera.get_position();

    ImGui::Text("fps: %f f/s", 1.0f / Context::event_handler().get_delta());
    ImGui::Text("delta: %fs", Context::event_handler().get_delta());
    ImGui::Text("pos: ( %.3f ; %.3f ; %.3f )", camera_pos.x, camera_pos.y, camera_pos.z);

    ImGui::End();

    ImGui::Begin("Scene");

    ImGui::Checkbox("Draw AABBs", &scene_graph.are_AABBs_drawn);
    ImGui::Text("Total Nodes Count: %lu", scene_graph.nodes.size());
    ImGui::Text("Total Drawn Objects: %lu", scene_graph.total_drawn_objects);

    ImGui::NewLine();
    ImGui::Checkbox("Draw Selected Mesh Normals", &scene_graph.are_normals_drawn);
    ImGui::Checkbox("Draw Selected Mesh Wireframe", &scene_graph.is_wireframe_drawn);

    ImGui::NewLine();
    scene_graph.add_imgui_node_tree();

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

    ImGui::Begin("Object Editor");
    scene_graph.add_object_editor_to_imgui_window();
    ImGui::End();
}
