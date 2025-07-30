#version 150 core

uniform sampler2D tex;

in vec3 normal_camera;
in vec2 frag_UV;

out vec4 outColor;

void main() {

    float c = dot(normal_camera, vec3(0.7, 0.7, 0)) * 0.5 + 0.5;

    outColor = texture(tex, frag_UV) * vec4(c, c, c, 1.0);
}