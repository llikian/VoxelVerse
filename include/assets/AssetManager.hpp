/***************************************************************************************************
 * @file  AssetManager.hpp
 * @brief Declaration of the AssetManager class
 **************************************************************************************************/

#pragma once

#include <functional>
#include "mesh/Mesh.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

enum ShaderName {
    SHADER_NONE = -1,

    SHADER_POINT_MESH,
    SHADER_LINE_MESH,
    SHADER_BACKGROUND,
    SHADER_FLAT,
    SHADER_LAMBERT,
    SHADER_BLINN_PHONG,
    SHADER_METALLIC_ROUGHNESS,
    SHADER_METALLIC_ROUGHNESS_NO_TANGENT,
    SHADER_TERRAIN,
    SHADER_POST_PROCESSING,
    SHADER_NORMALS,
    SHADER_WIREFRAME,

    SHADER_COUNT
};

/**
 * @class AssetManager
 * @brief
 */
class AssetManager {
public:
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    static inline AssetManager& get() {
        static AssetManager asset_manager;
        return asset_manager;
    }

    static Texture& add_texture(const std::filesystem::path& path, bool flip_vertically, bool srgb);
    static Texture& add_texture(const std::string& name, const Texture& texture);
    static Texture& add_texture(const std::string& name, const vec3& color);
    static Mesh& add_mesh(const std::string& name);

    template <typename MeshFunc, typename... Args>
    static Mesh& add_mesh(const std::string& name, MeshFunc&& create_mesh, Args&&... args) {
        Mesh& mesh = get().meshes.emplace(name, Mesh()).first->second;
        std::invoke(std::forward<MeshFunc>(create_mesh), mesh, std::forward<Args>(args)...);
        return mesh;
    }

    template <typename MeshFunc, typename... Args>
    static void add_two_meshes(const std::string& first,
                               const std::string& second,
                               MeshFunc&& create_mesh,
                               Args&&... args) {
        AssetManager& asset_manager = get();
        std::invoke(std::forward<MeshFunc>(create_mesh),
                    asset_manager.meshes.emplace(first, Mesh()).first->second,
                    asset_manager.meshes.emplace(second, Mesh()).first->second,
                    std::forward<Args>(args)...);
    }

    static Shader& get_shader(ShaderName shader_name);
    static Texture& get_texture(const std::string& texture_name_or_path);
    static Mesh& get_mesh(const std::string& mesh_name);

    static Texture* get_texture_ptr(const std::string& texture_name_or_path);
    static Mesh* get_mesh_ptr(const std::string& mesh_name);

    static bool has_texture(const std::string& texture_name_or_path);
    static bool has_mesh(const std::string& mesh_name);

    static Shader& get_relevant_shader_from_mesh(const Mesh& mesh);
    static ShaderName get_relevant_shader_name_from_mesh(const Mesh& mesh);

private:
    AssetManager();
    ~AssetManager();

    Shader shaders[SHADER_COUNT];
    std::unordered_map<std::string, Texture> textures;
    std::unordered_map<std::string, Mesh> meshes;
};
