#version 150 core

uniform sampler2D tex;

in vec2 frag_UV;

out vec4 outColor;

void main() {

    outColor = texture(tex, frag_UV);
}