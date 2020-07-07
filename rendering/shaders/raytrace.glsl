#version 450 core

/*
layout (std140) uniform ExampleBlock {
                        // Base Alignment  // Aligned Offset
    float value;        // 4                  0 
    vec3  vector;       // 16                 16  (vec3 have the same BA as a vec4)
    mat4  matrix;       // 16                 32  (col.1) (matrices are treated identically to an array of vectors)
                        // 16                 48  (col.1)
                        // 16                 64  (col.1)
                        // 16 (total: 64)     80  (col.1)
    float values[3];    // 16                 96  (val 0) (The BA of array elements are rounded up to the BA of a vec4)
                        // 16                 112 (val 1)
                        // 16 (total: 48)     128 (val 2)
    bool  boolean;      // 4                  144
    int   integer;      // 4                  148
}
*/
struct Vertex {
                    // Base Alignment  // Aligned Offset
    vec4 position;  // 4                  0
                    // 4                  4
                    // 4                  8
                    // 4 (total:16)       12
    vec4 normal;    // 4                  16
                    // 4                  20
                    // 4                  24
                    // 4 (total:16)       28
   
    vec2 tex_coord; // 4                  32 
                    // 4 (total:8)        36

    // (PADDING)    // 8                  40
    // (8 bytes of padding to pad out struct to a multiple of the size of a vec4 because it will be used in an array)

    // Total Size: 48
};

layout (std140, binding=0) buffer VertexBuffer {
    Vertex vertices[];
    //             // Base Alignment  // Aligned Offset
    // vertex[0]      48                 0
    // vertex[1]      48                 48
    // vertex[2]      48                 96
    // ...
    // Maximum of 2,666,666 Vertices (128 MB / 48 B)
} vertex_buffer;

// layout (std140, binding=0) buffer IndexBuffer {
//     int indices[];
// }

layout (binding = 0, rgba32f) uniform image2D framebuffer;

uniform vec3 eye;
uniform vec3 ray00;
uniform vec3 ray10;
uniform vec3 ray01;
uniform vec3 ray11;

struct Box {
    vec3 min;
    vec3 max;
};

#define NR_BOXES 2
const Box boxes[] = {
    /* Box in the middle */
    {vec3(-0.5, -0.5, -0.5), vec3(0.5, 0.5, 0.5)},
    /* The ground */
    {vec3(-5.0, -1.0, -5.0), vec3(5.0, -0.5, 5.0)},
};

vec2 intersect_box(vec3 origin, vec3 dir, const Box b) {
  vec3 t_min = (b.min - origin) / dir;
  vec3 t_max = (b.max - origin) / dir;
  vec3 t1 = min(t_min, t_max);
  vec3 t2 = max(t_min, t_max);
  float t_near = max(max(t1.x, t1.y), t1.z);
  float t_far = min(min(t2.x, t2.y), t2.z);
  return vec2(t_near, t_far);
}

#define FAR_PLANE 100.0f

vec3 intersect_boxes(vec3 eye, vec3 ray) {
    float min_t = FAR_PLANE;// * length(ray);
    for (int i=0; i<NR_BOXES; i++) {
        vec2 lambda = intersect_box(eye, ray, boxes[i]);
        if (lambda.x > 0.0 && lambda.x < lambda.y) {
            if (lambda.x <= min_t) {
                min_t = lambda.x;
            }
        }
    }

    return (min_t/FAR_PLANE).xxx;
}

layout (local_size_x = 8, local_size_y = 8) in;

void main() {
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(framebuffer);
    if (pix.x >= size.x || pix.y >= size.y) {
        return;
    }

    vec2 tex_coords = vec2(pix)/size;

    vec3 ray = mix(mix(ray00, ray10, tex_coords.x), mix(ray01, ray11, tex_coords.x), tex_coords.y);

    //imageStore(framebuffer, pix, vec4(intersect_boxes(eye, ray), 1.0f));
    vec4 col = vec4(vertex_buffer.vertices[1].normal.z, vertex_buffer.vertices[1].tex_coord.x, vertex_buffer.vertices[1].tex_coord.y, 1.0f);
    // vec4 col = vec4(0.2f,1.0f,0.5f,1.0f);
    imageStore(framebuffer, pix, col);
}