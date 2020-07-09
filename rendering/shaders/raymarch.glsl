#version 450 core

layout (binding = 0, rgba32f) uniform image2D framebuffer;

uniform vec3 eye;
uniform vec3 ray00;
uniform vec3 ray10;
uniform vec3 ray01;
uniform vec3 ray11;

#define EPSILON 0.000001f
#define NEAR_PLANE 0.1f
#define FAR_PLANE 100.0f
#define MIN_STEP_SIZE 0.001f
#define MAX_STEPS 500

#define NR_SPHERES 1

struct Sphere {
    vec3 position;
    float radius;
} spheres[] = {
    {vec3(0.0f, 0.0f, 0.0f), 2.0f},
    {vec3(3.0f, 3.5f, 0.0f), 1.0f},
    {vec3(-3.0f, 3.5f, 0.0f), 1.0f}
};

float sphere_SDF(vec3 point, Sphere sphere) {
    vec3 p = mod(point+5.0f, 10.0f)-5.0f;
    return length(sphere.position-p) - sphere.radius;
}

float floor_plane_SDF(vec3 point, float height) {
    return point.y-height;
}

#define POWER 8.0f
float mandelbulb_SDF(vec3 point) {
    vec3 z = point;
    float dr = 1.0f;
    float r = 0.0f;

    // vec4 trap = vec4(abs(z), length(z));

    for (int i=0; i<4; i++) {
        r = length(z);
        if (r>256.0f)
            break;
        
        float theta = acos(z.y/r);
        float phi = atan(z.x, z.z);
        dr = pow(r, POWER-1.0f) * POWER * dr + 1.0f;

        float zr = pow(r, POWER);
        theta *= POWER;
        phi *= POWER;

        z = zr*vec3(sin(theta)*sin(phi), cos(theta), sin(theta)*cos(phi));
        z += point;

        // trap = min(trap, vec4(abs(z), length(z)));
    }
    // return vec4(trap.yzw, 0.25*log(r)*r/dr);
    return 0.25*log(r)*r/dr;
}

float scene_SDF(vec3 point) {
    float dist = FAR_PLANE;
    // for (int j=0; j<NR_SPHERES; j++) {
    //     dist = min(dist, sphere_SDF(point, spheres[j]));
    // }
    // dist = min(dist, floor_plane_SDF(point, -1.0f));
    dist = min(dist, mandelbulb_SDF(point));
    return dist;
}

vec4 trace(vec3 ray_origin, vec3 ray_dir) {
    vec3 location = ray_origin;
    float dist = FAR_PLANE;
    for (int i=0; i<MAX_STEPS; i++) {
        // vec4 mandelbulb = mandelbulb_SDF(location);
        // dist = min(dist, mandelbulb.w);
        dist = scene_SDF(location);

        if (dist <= 0.0f) {
            // return vec4(mandelbulb.rgb, 1.0f);
            return vec4(location, 1.0f);
        }
        else if (dist >= FAR_PLANE-EPSILON) {
            break;
        }

        location += ray_dir*max(MIN_STEP_SIZE, dist);
    }
    return vec4(location, -1.0f);
}

#define OFFSET 0.0001f
#define SUN_DIR  normalize(vec3(-0.2f, 1.0f, 0.2f))
#define SUN_DIR2 normalize(vec3(0.2f, 1.0f, -0.2f))
#define BIAS 0.01f
vec4 shade(vec3 point, vec3 ray_dir) {
    vec3 normal = vec3(
        scene_SDF(vec3(point.x+OFFSET, point.y, point.z)) - scene_SDF(vec3(point.x-OFFSET, point.y, point.z)),
        scene_SDF(vec3(point.x, point.y+OFFSET, point.z)) - scene_SDF(vec3(point.x, point.y-OFFSET, point.z)),
        scene_SDF(vec3(point.x, point.y, point.z+OFFSET)) - scene_SDF(vec3(point.x, point.y, point.z-OFFSET))
    );
    normal = normalize(normal);
    vec3 diffuse = vec3(0.0f);
    if (trace(point+SUN_DIR*(0.01f), SUN_DIR).w <= 0.0f) {
        diffuse += dot(normal, SUN_DIR) * vec3(1.0f,0.5f,0.5f);
    }
    if (trace(point+SUN_DIR2*(0.01f), SUN_DIR2).w <= 0.0f) {
        diffuse += dot(normal, SUN_DIR2) * vec3(0.5f,1.0f,1.0f);
    }
    // vec3 halfway = normalize(normal + SUN_DIR);
    // float specular = pow(max(dot(normal, halfway), 0.0f), 128.0f);
    diffuse = max(diffuse, 0.1f.xxx);
    return vec4(diffuse, 1.0f);
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
    ray = normalize(ray);

    vec4 pos = trace(eye, ray);
    vec4 col;
    if (pos.w >= 0.0f) {
        col = shade(pos.xyz, ray);
    } else {
        col = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    imageStore(framebuffer, pix, col);
}