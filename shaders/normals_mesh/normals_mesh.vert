/***************************************************************************************************
 * @file  normals_mesh.vert
 * @brief Vertex shader for rendering a mesh's normals
 **************************************************************************************************/

#version 460 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;

out vec3 v_normal;

void main() {
    gl_Position = vec4(a_position, 1.0f);
    v_normal = a_normal;
}
