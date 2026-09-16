/***************************************************************************************************
 * @file  wireframe.geom
 * @brief Geometry shader for rendering a mesh's wireframe
 **************************************************************************************************/

#version 460 core

layout (triangles) in;
layout (line_strip, max_vertices = 4) out;

out vec3 v_color;

uniform mat4 u_mvp;

void main() {
    v_color = vec3(0.0f);

    gl_Position = u_mvp * gl_in[0].gl_Position;
    EmitVertex();

    gl_Position = u_mvp * gl_in[1].gl_Position;
    EmitVertex();

    gl_Position = u_mvp * gl_in[2].gl_Position;
    EmitVertex();

    gl_Position = u_mvp * gl_in[0].gl_Position;
    EmitVertex();

    EndPrimitive();
}