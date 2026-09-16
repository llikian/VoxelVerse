/***************************************************************************************************
 * @file  normals_mesh.geom
 * @brief Geometry shader for rendering a mesh's normals
 **************************************************************************************************/

#version 460 core

layout (points) in;
layout (line_strip, max_vertices = 2) out;

in vec3 v_normal[];
out vec3 v_color;

uniform mat4 u_mvp;
uniform float u_normal_length;

void main() {
    vec3 normal = normalize(v_normal[0]);
    v_color = 0.5f + 0.5f * normal;

    gl_Position = u_mvp * gl_in[0].gl_Position;
    EmitVertex();

    gl_Position = u_mvp * (gl_in[0].gl_Position + vec4(u_normal_length * normal, 0.0f));
    EmitVertex();

    EndPrimitive();
}