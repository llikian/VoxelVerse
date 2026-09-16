/***************************************************************************************************
 * @file  AssetManager.cpp
 * @brief Implementation of the AssetManager class
 **************************************************************************************************/

#include "assets/AssetManager.hpp"

#include <ranges>
#include "mesh/primitives.hpp"

Texture& AssetManager::add_texture(const std::filesystem::path& path, bool flip_vertically, bool srgb) {
    AssetManager& asset_manager = get();
    auto iterator = asset_manager.textures.find(path.string());

    if(iterator == asset_manager.textures.end()) {
        Texture texture;
        texture.create(path, flip_vertically, srgb);
        return asset_manager.textures.emplace(path.string(), texture).first->second;
    }

    return iterator->second;
}

Texture& AssetManager::add_texture(const std::string& name, const Texture& texture) {
    return get().textures.emplace(name, texture).first->second;
}

Texture& AssetManager::add_texture(const std::string& name, const vec3& color) {
    Texture texture;
    texture.create(color);
    return get().textures.emplace(name, texture).first->second;
}

Mesh& AssetManager::add_mesh(const std::string& name) {
    return get().meshes.emplace(name, Mesh()).first->second;
}

Shader& AssetManager::get_shader(ShaderName shader_name) {
    return get().shaders[shader_name];
}

Texture& AssetManager::get_texture(const std::string& texture_name_or_path) {
    AssetManager& asset_manager = get();

    auto iterator = asset_manager.textures.find(texture_name_or_path);
    if(iterator == asset_manager.textures.end()) {
        throw std::runtime_error("Couldn't find texture '" + texture_name_or_path + "' in asset manager");
    }

    return iterator->second;
}

Mesh& AssetManager::get_mesh(const std::string& mesh_name) {
    AssetManager& asset_manager = get();

    auto iterator = asset_manager.meshes.find(mesh_name);
    if(iterator == asset_manager.meshes.end()) {
        throw std::runtime_error("Couldn't find triangle mesh '" + mesh_name + "' in asset manager");
    }

    return iterator->second;
}

Texture* AssetManager::get_texture_ptr(const std::string& texture_name_or_path) {
    AssetManager& asset_manager = get();

    auto iterator = asset_manager.textures.find(texture_name_or_path);
    return iterator == asset_manager.textures.end() ? nullptr : &iterator->second;
}

Mesh* AssetManager::get_mesh_ptr(const std::string& mesh_name) {
    AssetManager& asset_manager = get();

    auto iterator = asset_manager.meshes.find(mesh_name);
    return iterator == asset_manager.meshes.end() ? nullptr : &iterator->second;
}

bool AssetManager::has_texture(const std::string& texture_name_or_path) {
    return get().textures.contains(texture_name_or_path);
}

bool AssetManager::has_mesh(const std::string& mesh_name) {
    return get().meshes.contains(mesh_name);
}

Shader& AssetManager::get_relevant_shader_from_mesh(const Mesh& mesh) {
    return get().shaders[get_relevant_shader_name_from_mesh(mesh)];
}

ShaderName AssetManager::get_relevant_shader_name_from_mesh(const Mesh& mesh) {
    switch(mesh.get_primitive()) {
        case MeshPrimitive::POINTS: return SHADER_POINT_MESH;
        case MeshPrimitive::LINES: return mesh.has_attribute(ATTRIBUTE_COLOR) ? SHADER_LINE_MESH : SHADER_FLAT;
        case MeshPrimitive::TRIANGLES: return mesh.has_attribute(ATTRIBUTE_NORMAL) ? SHADER_BLINN_PHONG : SHADER_FLAT;
        default: return SHADER_FLAT;
    }
}

AssetManager::AssetManager() {
    /* Shaders */
    shaders[SHADER_POINT_MESH].create({
                                          "shaders/point_mesh/point_mesh.vert",
                                          "shaders/point_mesh/point_mesh.frag"
                                      }, "point mesh");

    shaders[SHADER_LINE_MESH].create({
                                         "shaders/line_mesh/line_mesh.vert",
                                         "shaders/line_mesh/line_mesh.frag"
                                     }, "line mesh");

    shaders[SHADER_BACKGROUND].create({
                                          "shaders/vertex/position_only-no_mvp.vert",
                                          "shaders/fragment/background.frag"
                                      }, "background");

    shaders[SHADER_FLAT].create({
                                    "shaders/vertex/position_only.vert",
                                    "shaders/fragment/flat.frag"
                                }, "flat");

    shaders[SHADER_LAMBERT].create({
                                       "shaders/vertex/position_and_normal.vert",
                                       "shaders/fragment/lambert.frag"
                                   }, "lambert");

    shaders[SHADER_BLINN_PHONG].create({
                                           "shaders/vertex/default.vert",
                                           "shaders/fragment/blinn_phong.frag"
                                       }, "blinn-phong");

    shaders[SHADER_METALLIC_ROUGHNESS].create({
                                                  "shaders/vertex/tangent.vert",
                                                  "shaders/metallic-roughness/get_directions_tangent.frag",
                                                  "shaders/metallic-roughness/metallic_roughness.frag",
                                              }, "metallic-roughness");

    shaders[SHADER_METALLIC_ROUGHNESS_NO_TANGENT].create({
                                                             "shaders/vertex/default.vert",
                                                             "shaders/metallic-roughness/get_directions_no_tangent.frag",
                                                             "shaders/metallic-roughness/metallic_roughness.frag",
                                                         }, "metallic-roughness no tangent");

    shaders[SHADER_TERRAIN].create({
                                       "shaders/terrain/terrain.vert",
                                       "shaders/terrain/terrain.tesc",
                                       "shaders/terrain/terrain.tese",
                                       "shaders/terrain/terrain.frag"
                                   }, "terrain");

    shaders[SHADER_POST_PROCESSING].create({
                                               "shaders/vertex/position_only-no_mvp.vert",
                                               "shaders/fragment/post_processing.frag"
                                           }, "post processing");

    shaders[SHADER_NORMALS].create({
                                       "shaders/normals_mesh/normals_mesh.vert",
                                       "shaders/normals_mesh/normals_mesh.geom",
                                       "shaders/line_mesh/line_mesh.frag"
                                   }, "normals mesh");

    shaders[SHADER_WIREFRAME].create({
                                       "shaders/vertex/position_only-no_mvp.vert",
                                       "shaders/wireframe/wireframe.geom",
                                       "shaders/line_mesh/line_mesh.frag"
                                   }, "wireframe");
}

AssetManager::~AssetManager() {
    for(unsigned int i = 0 ; i < SHADER_COUNT ; ++i) { shaders[i].free(); }
    for(Texture& texture : textures | std::views::values) { texture.free(); }
}
