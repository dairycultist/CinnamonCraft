#version 150 core

uniform mat4 position_matrix;

in vec3 position;
in vec2 UV;

out vec2 frag_UV;

void main() {

    // get final position
    gl_Position = position_matrix * vec4(position.xy, -position.z, 1.0);

    // pass along UV
    frag_UV = UV;
}