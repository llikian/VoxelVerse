/***************************************************************************************************
 * @file  Node.hpp
 * @brief Declaration of the Node struct
 **************************************************************************************************/

#pragma once

#include <string>
#include <vector>

#include "assets/AssetManager.hpp"

constexpr std::size_t INVALID_INDEX = ~0uz;

struct Node {
    enum class Type : unsigned char {
        SIMPLE,     // NO DATA
        MESH,       // MESH, SHADER, (COLOR), (MATERIAL)
        GLTF_SCENE, // MESH, SHADER
    };

    Node(const std::string& name, std::size_t parent, Type type);

    Type type;        ///< The type of the node.
    std::string name; ///< The name of the node.

    std::size_t parent;                ///< The index of the node's parent. INVALID_INDEX if it is the root.
    std::vector<std::size_t> children; ///< The indices of the node's children.

    std::size_t drawable_index; ///< The index of node's drawable object. INVALID_INDEX if not drawable.
    ShaderName shader_name;     ///< The name of the node's shader. SHADER_NONE if no shader.
    std::size_t color_index;    ///< The index of the node's color. INVALID_INDEX if no color.
    std::size_t scene_index;    ///< The index of the node's scene. INVALID_INDEX if not a scene.
    std::size_t material_index; ///< The index of the node's material. INVALID_INDEX if no material.

    bool is_visible;  ///< Whether the node is visible.
    bool is_selected; ///< Whether the node is selected.
};
