/***************************************************************************************************
 * @file  Node.cpp
 * @brief Implementation of the Node struct
 **************************************************************************************************/

#include "engine/Node.hpp"

Node::Node(const std::string& name, std::size_t parent, Type type)
    : type(type),
      name(name),
      parent(parent),
      drawable_index(INVALID_INDEX),
      shader_name(SHADER_NONE),
      color_index(INVALID_INDEX),
      scene_index(INVALID_INDEX),
      material_index(INVALID_INDEX),
      is_visible(true),
      is_selected(false) {}
