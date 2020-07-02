#version 450

layout(location=0) out vec4 frag_color;

in vec2 texture_coordinate;

void main() {
    frag_color = vec4(texture_coordinate, 0.0f, 1.0f);
    // frag_color.xyz = pow(frag_color.xyz, vec3(1.0f/2.2f));
}