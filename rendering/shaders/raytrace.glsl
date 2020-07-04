#version 450 core

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

    imageStore(framebuffer, pix, vec4((intersect_boxes(eye, ray)), 1.0f));
}