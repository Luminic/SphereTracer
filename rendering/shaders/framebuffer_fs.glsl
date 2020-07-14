#version 450

layout(location=0) out vec4 frag_color;

in vec2 texture_coordinate;

uniform sampler2D render;

void main() {
    vec3 col = texture(render, texture_coordinate).rgb;
    frag_color = vec4(pow(col, 1/2.2f.xxx), 1.0f);
}