/***************************************************************************************************
 * @file  SceneGraph.cpp
 * @brief Implementation of the SceneGraph class
 **************************************************************************************************/

#include "engine/SceneGraph.hpp"

#include "assets/AssetManager.hpp"
#include "culling/Ray.hpp"
#include "engine/Context.hpp"
#include "engine/Node.hpp"
#include "maths/geometry.hpp"
#include "mesh/primitives.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"

bool is_mouse_hovering_imgui() {
    ImGuiContext* imgui_context = ImGui::GetCurrentContext();
    return imgui_context->HoveredWindow != nullptr &&
           (imgui_context->HoveredWindow->Flags & ImGuiWindowFlags_NoMouseInputs) == 0;
}

SceneGraph::SceneGraph(Camera& camera)
    : are_AABBs_drawn(false),
      are_normals_drawn(false),
      is_wireframe_drawn(true),
      active_camera(&camera),
      light_node_index(INVALID_INDEX),
      selected_node(INVALID_INDEX),
      should_open_add_node_popup(false),
      add_node_child_parent(INVALID_INDEX),
      should_open_rename_popup(false),
      rename_target(INVALID_INDEX) {
    /* ---- Asset Manager ---- */
    /* Meshes */
    AssetManager::add_mesh("sphere 8 16", create_sphere_mesh, 8, 16);
    AssetManager::add_mesh("sphere 16 32", create_sphere_mesh, 16, 32);
    AssetManager::add_mesh("icosphere 0", create_icosphere_mesh, 0);
    AssetManager::add_mesh("icosphere 1", create_icosphere_mesh, 1);
    AssetManager::add_mesh("icosphere 2", create_icosphere_mesh, 2);
    AssetManager::add_mesh("cube", create_cube_mesh);
    AssetManager::add_mesh("wireframe cube", create_wireframe_cube_mesh);
    AssetManager::add_mesh("screen", create_screen_mesh);
    AssetManager::add_mesh("axes", create_axes_mesh, 0.5f);
    AssetManager::add_mesh("camera pyramid",
                           create_pyramid_mesh,
                           vec3(1.0f, 1.0f, -1.0f),
                           vec3(1.0f, -1.0f, -1.0f),
                           vec3(-1.0f, -1.0f, -1.0f),
                           1.0f);

    /* Textures */
    AssetManager::add_texture("default", vec3(1.0f));
    AssetManager::add_texture("red", vec3(1.0f, 0.0f, 0.0f));
    AssetManager::add_texture("green", vec3(0.0f, 1.0f, 0.0f));
    AssetManager::add_texture("blue", vec3(0.0f, 0.0f, 1.0f));

    /* ---- Root ---- */
    add_simple_node("Scene Graph", INVALID_INDEX);

    /* ---- Light ---- */
    light_node_index = add_mesh_node("Light", 0, AssetManager::get_mesh_ptr("icosphere 1"), SHADER_FLAT);
    add_color_to_node(light_node_index, vec4(1.0f));
    transforms[light_node_index].set_local_position(0.0f, 10.0f, 0.0f);

    /* Event Handler */
    Context::event_handler().set_left_click_func([this] {
        Context* context = Context::context();
        if(AABBs.size() > 1 && context->is_main_window_hovered && context->event_handler().is_cursor_visible()) {
            if(selected_node != INVALID_INDEX) { set_is_selected(selected_node, false); }
            selected_node = INVALID_INDEX;

            /* Create Ray */
            vec2 mouse_pos = context->mouse_pos_in_main_window;
            vec2 window_resolution = context->framebuffer_resolution;

            vec2 normalized_mouse_pos = (mouse_pos / window_resolution) * 2.0f - vec2(1.0f);
            normalized_mouse_pos.y = -normalized_mouse_pos.y; // Flip Y because window origin is in top left corner.

            mat4 vp_inverse = active_camera->get_inverse_view_projection_matrix();

            Ray ray(vp_inverse * vec4(normalized_mouse_pos, -1.0f, 1.0f),
                    vp_inverse * vec4(normalized_mouse_pos, 1.0f, 1.0f));

            /* Intersect AABBs */
            std::vector<std::size_t> intersected_indices;
            intersected_indices.reserve(8);

            for(std::size_t i = 0; i < nodes.size(); ++i) {
                float tmin, tmax;
                if(nodes[i].type == Node::Type::MESH && ray.intersect_aabb(AABBs[i], tmin, tmax)) {
                    // Ray hits AABB from outside OR ray origin is inside the AABB
                    if(tmin > 0.0f || (tmin <= 0.0f && tmax >= 0.0f)) { intersected_indices.push_back(i); }
                }
            }

            /* Intersect Meshes */
            float distance = infinity;
            for(std::size_t index : intersected_indices) {
                float dist = meshes[nodes[index].drawable_index]->intersect(
                    ray,
                    transforms[index].get_global_model_const_reference());
                if(dist > 0.0f && dist < distance) {
                    distance = dist;
                    selected_node = index;
                }
            }

            if(selected_node != INVALID_INDEX) { set_is_selected(selected_node, true); }
        }
    });
}

Node& SceneGraph::operator[](std::size_t node_index) {
    return nodes[node_index];
}

void SceneGraph::draw(const Frustum& frustum) {
    total_drawn_objects = 0;

    const vec4& color = colors[nodes[light_node_index].color_index];
    light_color.x = color.x;
    light_color.y = color.y;
    light_color.z = color.z;
    light_position = transforms[light_node_index].get_global_position();

    update_transform_and_children();
    update_AABBs();

    draw(frustum, 0);

    if(selected_node != INVALID_INDEX && nodes[selected_node].type == Node::Type::MESH) {
        mat4 mvp = frustum.view_projection * transforms[selected_node].get_global_model_const_reference();
        const Mesh* mesh = meshes[nodes[selected_node].drawable_index];

        if(are_normals_drawn) {
            static const Shader& normals_shader = AssetManager::get_shader(SHADER_NORMALS);
            normals_shader.use();
            normals_shader.set_uniform("u_mvp", mvp);
            const AABB& aabb = AABBs[selected_node];
            float dist =
                0.05f * std::log(10.0f * aabb.get_size()) * length(aabb.get_center() - active_camera->get_position());
            normals_shader.set_uniform("u_normal_length", dist * std::tan(active_camera->get_fov() * 0.5f));
            mesh->draw_normals();
        }

        if(is_wireframe_drawn && !Context::event_handler().is_wireframe_enabled()) {
            static const Shader& wireframe_shader = AssetManager::get_shader(SHADER_WIREFRAME);
            wireframe_shader.use();
            wireframe_shader.set_uniform("u_mvp", mvp);
            mesh->draw_wireframe();
        }
    }
}

std::size_t SceneGraph::add_simple_node(const std::string& name, std::size_t parent) {
    return add_node(name, parent, Node::Type::SIMPLE);
}

std::size_t
SceneGraph::add_mesh_node(const std::string& name, std::size_t parent, std::size_t mesh_index, ShaderName shader_name) {
    std::size_t index = add_node(name, parent, Node::Type::MESH);

    nodes[index].drawable_index = mesh_index;
    nodes[index].shader_name = shader_name;

    return index;
}

std::size_t
SceneGraph::add_mesh_node(const std::string& name, std::size_t parent, const Mesh* mesh, ShaderName shader_name) {
    meshes.push_back(mesh);

    return add_mesh_node(name, parent, meshes.size() - 1, shader_name);
}

std::size_t
SceneGraph::add_gltf_scene_node(const std::string& name, std::size_t parent, const std::filesystem::path& scene_path) {
    std::size_t index = add_node(name, parent, Node::Type::GLTF_SCENE);

    gltf_scenes.emplace_back(scene_path, this, index);
    nodes[index].scene_index = gltf_scenes.size() - 1;

    return index;
}

std::size_t SceneGraph::add_mesh(const Mesh* mesh) {
    meshes.push_back(mesh);
    return meshes.size() - 1;
}

std::size_t SceneGraph::add_color_to_node(std::size_t node_index, const vec4& color) {
    colors.push_back(color);
    std::size_t color_index = colors.size() - 1;
    nodes[node_index].color_index = color_index;
    return color_index;
}

std::size_t SceneGraph::add_material_to_node(std::size_t node_index, Material* material) {
    materials.push_back(material);
    std::size_t material_index = materials.size() - 1;
    nodes[node_index].material_index = material_index;
    return material_index;
}

void SceneGraph::add_imgui_node_tree() {
    add_node_to_imgui_node_tree(0);

    if(should_open_add_node_popup) {
        ImGui::OpenPopup("Add Simple Node");
        should_open_add_node_popup = false;
    }

    if(ImGui::BeginPopup("Add Simple Node", ImGuiWindowFlags_AlwaysAutoResize)) {
        bool confirmed = ImGui::InputText("##add_node", &add_node_child_name, ImGuiInputTextFlags_EnterReturnsTrue);

        if(ImGui::Button("OK") || confirmed) {
            add_simple_node(add_node_child_name, add_node_child_parent);
            add_node_child_parent = INVALID_INDEX;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if(ImGui::Button("Cancel")) {
            add_node_child_parent = INVALID_INDEX;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    } else {
        add_node_child_parent = INVALID_INDEX;
    }

    if(should_open_rename_popup) {
        ImGui::OpenPopup("Rename Node");
        should_open_rename_popup = false;
    }

    if(ImGui::BeginPopup("Rename Node", ImGuiWindowFlags_AlwaysAutoResize)) {
        bool confirmed = ImGui::InputText("##rename", &rename_string, ImGuiInputTextFlags_EnterReturnsTrue);

        if(ImGui::Button("OK") || confirmed) {
            nodes[rename_target].name = rename_string;
            rename_target = INVALID_INDEX;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if(ImGui::Button("Cancel")) {
            rename_target = INVALID_INDEX;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    } else {
        rename_target = INVALID_INDEX;
    }

    /* List Meshes */
    ImGui::NewLine();
    static bool are_meshes_shown = false;
    if(are_meshes_shown) {
        if(ImGui::Button("Meshes v")) { are_meshes_shown = !are_meshes_shown; }
    } else {
        if(ImGui::Button("Meshes >")) { are_meshes_shown = !are_meshes_shown; }
    }
    if(are_meshes_shown) {
        ImGui::BeginTable("Meshes", 3);

        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Vertices");
        ImGui::TableSetupColumn("Indices");
        ImGui::TableHeadersRow();

        for(std::size_t i = 0; i < meshes.size(); ++i) {

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%zu", i);
            ImGui::TableNextColumn();
            ImGui::Text("%zu", meshes[i]->get_vertices_amount());
            ImGui::TableNextColumn();
            ImGui::Text("%zu", meshes[i]->get_indices_amount());
        }

        ImGui::EndTable();
    }

    ImGui::NewLine();
    static bool are_materials_shown = false;
    if(are_materials_shown) {
        if(ImGui::Button("Materials v")) { are_materials_shown = !are_materials_shown; }
    } else {
        if(ImGui::Button("Materials >")) { are_materials_shown = !are_materials_shown; }
    }
    if(are_materials_shown) {
        ImGui::BeginTable("Materials", 2);

        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Name");
        ImGui::TableHeadersRow();

        for(std::size_t i = 0; i < materials.size(); ++i) {

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%zu", i);
            ImGui::TableNextColumn();
            ImGui::Text("%s", materials[i]->name.c_str());
        }

        ImGui::EndTable();
    }
}

void SceneGraph::add_object_editor_to_imgui_window() {
    if(selected_node < nodes.size()) {
        Node& node = nodes[selected_node];
        Transform& transform = transforms[selected_node];

        ImGui::Text("Node");
        ImGui::SameLine();
        ImGui::PushID(&node.name);
        ImGui::InputText("", &node.name);
        ImGui::PopID();
        ImGui::Text("ID: %zu", selected_node);
        ImGui::NewLine();

        if(ImGui::Button("UNSELECT")) {
            set_is_selected(selected_node, false);
            selected_node = INVALID_INDEX;
        }

        if(ImGui::Checkbox("Is Object Visible", &node.is_visible)) { set_visibility(selected_node, node.is_visible); }

        bool is_dirty = ImGui::DragFloat3("Local Position", &transform.get_local_position_reference().x);

        quaternion& orientation = transform.get_local_orientation_reference();
        if(ImGui::DragFloat4("Local Orientation", &orientation.x, 0.1f)) {
            is_dirty = true;
            orientation.normalize();
        }

        is_dirty = is_dirty || ImGui::DragFloat3("Local Scale", &transform.get_local_scale_reference().x, 0.1f, 0.1f);

        if(is_dirty) { transform.set_local_model_to_dirty(); }

        ImGui::NewLine();
        switch(node.type) {
            case Node::Type::MESH: {
                const Mesh* mesh = meshes[node.drawable_index];
                ImGui::Text("Mesh: %zu vertices, %zu indices, attributes:",
                            mesh->get_vertices_amount(),
                            mesh->get_indices_amount());
                for(unsigned int i = 0; i < ATTRIBUTE_AMOUNT; ++i) {
                    Attribute attribute = static_cast<Attribute>(i);
                    if(mesh->has_attribute(attribute)) {
                        ImGui::Text(" - %s (%s)",
                                    attribute_to_string(attribute).c_str(),
                                    attribute_type_to_string(mesh->get_attribute_type(attribute)).c_str());
                    }
                }
                break;
            }
            default: break;
        }

        ImGui::NewLine();
        if(node.shader_name != SHADER_NONE) {
            ImGui::Text("Shader: '%s'", AssetManager::get_shader(node.shader_name).get_name().c_str());
        }
        if(node.color_index != INVALID_INDEX) { ImGui::ColorEdit4("Color", &colors[node.color_index].x); }
        if(node.material_index != INVALID_INDEX) { materials[node.material_index]->add_to_object_editor(); }
    } else {
        ImGui::Text("No Node is Selected");
    }
}

void SceneGraph::set_visibility(std::size_t node_index, bool is_visible) {
    nodes[node_index].is_visible = is_visible;
    for(std::size_t index : nodes[node_index].children) { set_visibility(index, is_visible); }
}

void SceneGraph::set_is_selected(std::size_t node_index, bool is_selected) {
    nodes[node_index].is_selected = is_selected;
    for(std::size_t index : nodes[node_index].children) { set_is_selected(index, is_selected); }
}

bool SceneGraph::is_ancestor_of(std::size_t potential_parent, std::size_t node_index) {
    if(node_index == INVALID_INDEX || nodes[node_index].parent == INVALID_INDEX) { return false; }
    if(potential_parent == 0) { return true; }

    while(nodes[node_index].parent != INVALID_INDEX) {
        if(nodes[node_index].parent == potential_parent) { return true; }
        node_index = nodes[node_index].parent;
    }

    return false;
}

void SceneGraph::draw(const Frustum& frustum, std::size_t node_index) {
    const Node& node = nodes[node_index];

    if(!node.is_visible) { return; }

    if(AABBs[node_index].is_in_frustum(frustum)) {
        if(node.drawable_index != INVALID_INDEX) {
            ++total_drawn_objects;
            draw(frustum.view_projection, AssetManager::get_shader(node.shader_name), node_index);
        }

        if(are_AABBs_drawn || node.is_selected) {
            const Shader& shader = AssetManager::get_shader(SHADER_FLAT);
            shader.use();
            shader.set_uniform("u_mvp", frustum.view_projection * AABBs[node_index].get_global_model_matrix());

            if(node.is_selected) {
                if(node.parent == INVALID_INDEX || !nodes[node.parent].is_selected) {
                    shader.set_uniform("u_color", vec4(0.0f, 1.0f, 1.0f, 1.0f));
                } else {
                    shader.set_uniform("u_color", vec4(0.0f, 0.0f, 1.0f, 1.0f));
                }
            } else if(node.drawable_index == INVALID_INDEX) { // Not a drawable node.
                shader.set_uniform("u_color", vec4(0.0f, 1.0f, 0.0f, 1.0f));
            } else {
                shader.set_uniform("u_color", vec4(1.0f, 0.0f, 0.0f, 1.0f));
            }

            glLineWidth(3.0f);
            AssetManager::get_mesh("wireframe cube").draw();
            glLineWidth(1.0f);
        }

        for(std::size_t index : node.children) { draw(frustum, index); }
    }
}

void SceneGraph::draw(const mat4& view_projection, const Shader& shader, std::size_t node_index) const {
    const Node& node = nodes[node_index];

    shader.use();

    const mat4& global_model = transforms[node_index].get_global_model_const_reference();
    shader.set_uniform_if_exists("u_model", global_model);

    int u_mvp_location = shader.get_uniform_location("u_mvp");
    if(u_mvp_location != -1) { Shader::set_uniform(u_mvp_location, view_projection * global_model); }

    int u_normals_model_matrix_location = shader.get_uniform_location("u_normals_model_matrix");
    if(u_normals_model_matrix_location != -1) {
        Shader::set_uniform(u_normals_model_matrix_location, transpose_inverse(global_model));
    }

    shader.set_uniform_if_exists("u_light.intensity", 3.0f);
    shader.set_uniform_if_exists("u_light.color", light_color);
    shader.set_uniform_if_exists("u_light.position", light_position);
    shader.set_uniform_if_exists("u_camera_position", active_camera->get_position());

    if(node.color_index != INVALID_INDEX) { shader.set_uniform_if_exists("u_color", colors[node.color_index]); }
    if(node.material_index != INVALID_INDEX) { materials[node.material_index]->update_shader_uniforms(&shader); }

    switch(node.type) {
        case Node::Type::MESH: meshes[node.drawable_index]->draw(); break;
        default:               break;
    }
}

void SceneGraph::update_transform_and_children(std::size_t node_index) {
    if(transforms[node_index].is_local_model_dirty()) {
        force_update_transform_and_children(node_index);
    } else {
        for(std::size_t child : nodes[node_index].children) { update_transform_and_children(child); }
    }
}

void SceneGraph::force_update_transform_and_children(std::size_t node_index) {
    Node& node = nodes[node_index];
    if(nodes[node_index].parent != INVALID_INDEX) {
        transforms[node_index].update_global_model(transforms[node.parent].get_global_model_const_reference());
    } else {
        transforms[node_index].update_global_model();
    }

    for(std::size_t child : nodes[node_index].children) { force_update_transform_and_children(child); }
}

void SceneGraph::update_AABBs(std::size_t node_index) {
    for(std::size_t index : nodes[node_index].children) { update_AABBs(index); }

    vec3 min(std::numeric_limits<float>::max());
    vec3 max(std::numeric_limits<float>::lowest());

    switch(nodes[node_index].type) {
        case Node::Type::MESH:
            AABBs[node_index].set(meshes[nodes[node_index].drawable_index]->get_AABB(), transforms[node_index]);
            break;
        case Node::Type::SIMPLE:
        case Node::Type::GLTF_SCENE:
            for(std::size_t index : nodes[node_index].children) {
                AABB::axis_aligned_min(min, AABBs[index].min_point);
                AABB::axis_aligned_max(max, AABBs[index].max_point);
            }
            AABBs[node_index].set(min, max);
            break;
    }
}

std::size_t SceneGraph::add_node(const std::string& name, std::size_t parent, Node::Type type) {
    if(freed_indices.empty()) {
        nodes.emplace_back(name, parent, type);
        is_alive.push_back(true);
        transforms.emplace_back();
        AABBs.emplace_back();
        is_in_frustum.push_back(false);

        std::size_t index = nodes.size() - 1;
        if(parent != INVALID_INDEX) { nodes[parent].children.push_back(index); }

        return nodes.size() - 1;
    } else {
        std::size_t node_index = freed_indices.back();
        freed_indices.pop_back();

        nodes[node_index] = Node(name, parent, type);
        is_alive[node_index] = true;
        transforms[node_index] = Transform();
        AABBs[node_index] = AABB();
        is_in_frustum[node_index] = false;

        if(parent != INVALID_INDEX) { nodes[parent].children.push_back(node_index); }

        return node_index;
    }
}

void SceneGraph::remove_node(std::size_t node_index) {
    for(std::size_t child_index : nodes[node_index].children) { set_node_to_dead(child_index); }

    remove_child_from_parent(node_index);
    is_alive[node_index] = false;
    freed_indices.push_back(node_index);
}

void SceneGraph::set_node_to_dead(std::size_t node_index) {
    for(std::size_t child_index : nodes[node_index].children) { set_node_to_dead(child_index); }

    is_alive[node_index] = false;
    freed_indices.push_back(node_index);
}

void SceneGraph::remove_child_from_parent(std::size_t node_index) {
    if(nodes[node_index].parent != INVALID_INDEX) {
        std::vector<std::size_t>& parent_children = nodes[nodes[node_index].parent].children;

        auto ite = std::find(parent_children.begin(), parent_children.end(), node_index);
        if(ite != parent_children.end()) {
            parent_children.erase(ite);
        } else {
            std::cout << node_index << '\n';
            throw std::runtime_error("Node index not found in old parent's children");
        }
    }
}

void SceneGraph::add_node_to_imgui_node_tree(std::size_t node_index) {
    if(!is_alive[node_index]) { return; }

    const Node& node = nodes[node_index];

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;
    if(node.children.empty()) { flags |= ImGuiTreeNodeFlags_Leaf; }
    if(selected_node == node_index) { flags |= ImGuiTreeNodeFlags_Selected; }

    std::string label;
    switch(node.type) {
        case Node::Type::SIMPLE:     label += " o"; break;
        case Node::Type::MESH:       label += " M"; break;
        case Node::Type::GLTF_SCENE: label += " S"; break;
        default:                     label += " ?"; break;
    }
    label += ' ' + node.name;

    ImGui::PushID(&node);
    if(ImGui::TreeNodeEx(label.c_str(), flags)) {
        if(ImGui::IsItemClicked()) {
            if(selected_node != INVALID_INDEX) { set_is_selected(selected_node, false); }
            selected_node = node_index;
            set_is_selected(selected_node, true);
        }

        /* Right Click Menu */
        if(ImGui::BeginPopupContextItem()) {
            if(ImGui::MenuItem("Add Simple Node")) {
                should_open_add_node_popup = true;
                add_node_child_parent = node_index;
                add_node_child_name = "New Simple Node";
            }

            if(ImGui::MenuItem("Rename")) {
                should_open_rename_popup = true;
                rename_target = node_index;
                rename_string = node.name;
            };

            if(node_index != 0 && ImGui::MenuItem("Delete")) { remove_node(node_index); }

            ImGui::EndPopup();
        }

        /* Drag & Drop Nodes */
        if(node_index != 0 && ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("SCENE_GRAPH_NODE", &node_index, sizeof(std::size_t));
            ImGui::Text("%s", label.c_str());
            ImGui::EndDragDropSource();
        }

        if(ImGui::BeginDragDropTarget()) {
            if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_GRAPH_NODE")) {
                std::size_t dragged_index = *static_cast<const std::size_t*>(payload->Data);

                if(dragged_index != node_index                  //
                   && node_index != nodes[dragged_index].parent //
                   && !is_ancestor_of(dragged_index, node_index)) {
                    reparent_node(dragged_index, node_index);
                }
            }
            ImGui::EndDragDropTarget();
        }

        for(std::size_t index : node.children) { add_node_to_imgui_node_tree(index); }
        ImGui::TreePop();
    } else if(ImGui::IsItemClicked()) {
        if(selected_node != INVALID_INDEX) { set_is_selected(selected_node, false); }
        selected_node = node_index;
        set_is_selected(selected_node, true);
    }
    ImGui::PopID();
}

void SceneGraph::reparent_node(std::size_t node_index, std::size_t new_parent_index) {
    remove_child_from_parent(node_index);

    nodes[new_parent_index].children.push_back(node_index);
    nodes[node_index].parent = new_parent_index;

    force_update_transform_and_children(node_index);
    update_AABBs(node_index);
}
