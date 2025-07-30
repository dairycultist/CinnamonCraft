#version 150 core

uniform mat4 position_matrix;
uniform mat4 normal_matrix;

in vec3 position;
in vec3 normal;
in vec2 UV;

out vec3 normal_camera;
out vec2 frag_UV;

void main() {

    // get final position
    gl_Position = position_matrix * vec4(position.xy, -position.z, 1.0);

    // get final normal
    normal_camera = (normal_matrix * vec4(normal, 1.0)).xyz;

    // pass along UV
    frag_UV = UV;
}