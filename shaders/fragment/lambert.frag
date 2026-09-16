/***************************************************************************************************
 * @file  phong.frag
 * @brief Fragment shader implementing a basic lambertian diffuse
 **************************************************************************************************/

#version 460 core

in vec3 v_position;
in vec3 v_normal;

out vec4 frag_color;

struct Light {
    vec3 color;
    vec3 position;
};

uniform Light u_light;
uniform vec4 u_color;

void main() {
    float ambient = 0.2f;
    float diffuse = max(dot(normalize(v_normal), normalize(u_light.position - v_position)), 0.0f);

    frag_color = vec4(u_color.rgb * (ambient + diffuse * u_light.color), u_color.a);
}
