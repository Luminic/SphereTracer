#version 450 core

layout (binding = 0, rgba32f) uniform image2D framebuffer;

uniform vec3 eye;
uniform vec3 ray00;
uniform vec3 ray10;
uniform vec3 ray01;
uniform vec3 ray11;

// struct box {
//     vec3 min;
//     vec3 max;
// };

layout (local_size_x = 8, local_size_y = 8) in;

void main() {
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(framebuffer);
    if (pix.x >= size.x || pix.y >= size.y) {
        return;
    }
    imageStore(framebuffer, pix, vec4(0.0f,0.0f,1.0f,0.0f));
}