#version 450

layout(location=0) out vec4 frag_color;

in vec2 texture_coordinate;

uniform sampler2D render;

void main() {
    frag_color = vec4(texture(render, texture_coordinate).rgb, 1.0f);
}