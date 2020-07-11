#version 450 core

layout (binding = 0, rgba32f) uniform image2D framebuffer;

uniform vec3 eye;
uniform vec3 ray00;
uniform vec3 ray10;
uniform vec3 ray01;
uniform vec3 ray11;

uniform int time;

#define EPSILON 0.000001f
#define NEAR_PLANE 0.1f
#define FAR_PLANE 100.0f
#define MIN_STEP_SIZE 0.0001f
#define MAX_STEPS 500

struct Material {
    vec3 color;
};


#define NR_SPHERES 3
struct Sphere {
    vec3 position;
    float radius;
} spheres[] = {
    {vec3(3.0f, 0.0f, 0.0f), 1.5f},
    {vec3(3.0f, 3.5f, 0.0f), 1.0f},
    {vec3(-3.0f, 3.5f, 0.0f), 1.0f}
};

float sphere_SDF(vec3 point, Sphere sphere) {
    return length(point-sphere.position) - sphere.radius;
}

struct Cylinder {
    vec3 position;
    float height;
    float radius;
};

float cylinder_SDF(vec3 point, Cylinder cylinder) {
    vec3 p = point - cylinder.position;
    vec2 d = abs(vec2(length(p.xz),p.y)) - vec2(cylinder.height, cylinder.radius);
    return min(max(d.x, d.y), 0.0f) + length(max(d, 0.0f));
}

float floor_plane_SDF(vec3 point, float height) {
    return point.y-height;
}

#define POWER 8
#define ITERATIONS 4
#define BAILOUT 256.0f
float mandelbulb_SDF(vec3 point, out Material material) {
    vec3 z = point;
    float dr = 1.0f;
    float r = 0.0f;

    vec4 trap = vec4(abs(z), dot(z,z));

    for (int i=0; i<ITERATIONS; i++) {
        r = length(z);
        if (r*r>BAILOUT)
            break;

        dr = pow(r, POWER-1.0f) * POWER * dr + 1.0f;
        float zr = pow(r, POWER);

        trap = max(trap, vec4(abs(z), dot(z,z)));
        #if POWER == 8
            // If the power is 8 we can optimize out the inv trig and trig functions by applying
            // the double angle formula 3 times
            
            float len_xz = length(z.xz);

            float sin_theta = z.z / len_xz;
            float cos_theta = z.x / len_xz;

            float sin_phi = len_xz / r;
            float cos_phi = z.y / r;

            float cos_theta_squared = pow(z.x, 2) / dot(z.xz, z.xz);
            float cos_phi_squared = pow(z.y, 2) / dot(z, z);

            float sin_8_phi = 2*(2*(2*sin_phi*cos_phi)*(2*cos_phi_squared-1))*(2*pow(2*cos_phi_squared-1, 2)-1);

            z = zr * vec3(
            //  cos(8*theta)                                   // sin(8*phi)
                (2*pow(2*pow(2*cos_theta_squared-1, 2)-1, 2)-1) * sin_8_phi,
            //  cos(8*phi)
                (2*pow(2*pow(2*cos_phi_squared-1, 2)-1, 2)-1),
            //  sin(8*theta)                                           // sin(8*phi)
                (2*(2*(2*sin_theta*cos_theta)*(2*cos_theta_squared-1))) * (2*pow(2*cos_theta_squared-1, 2)-1) * sin_8_phi
            );
            z += point;
        #else
            float theta = atan(z.z, z.x);
            float phi = acos(z.y/r);
            theta *= POWER;
            phi *= POWER;
            z = zr*vec3(cos(theta)*sin(phi), cos(phi), sin(theta)*sin(phi));
            z += point;
        #endif
    }
    material = Material(trap.xyz);
    return 0.25*log(dot(z,z))*length(z)/dr;
}

float scene_SDF(vec3 point, out Material material) {
    float dist = FAR_PLANE;
    Material tmp;
    float d;

    d = mandelbulb_SDF(point, tmp);
    if (d <= dist) {
        dist = d;
        material = tmp;
    }

    for (int j=0; j<NR_SPHERES; j++) {
        d = sphere_SDF(point, spheres[j]);
        if (d <= dist) {
            dist = d;
            material = Material(0.8f.xxx);
        }
    }

    d = cylinder_SDF(point, Cylinder(vec3(-3.0f, 0.0f, 0.0f), 0.5f, 1.5));
    if (d <= dist) {
        dist = d;
        material = Material(0.5f.xxx);
    }

    // Floor plane
    d = max(point.y+1.5f, -1.6f-point.y);
    if (d <= dist) {
        dist = d;
        material = Material(1.0f.xxx);
    }
    return dist;
}

vec4 trace(vec3 ray_origin, vec3 ray_dir, out Material material) {
    vec3 location = ray_origin;
    float dist = FAR_PLANE;
    for (int i=0; i<MAX_STEPS; i++) {
        dist = scene_SDF(location, material);

        if (dist <= 0.0f) {
            return vec4(location, 1.0f);
            // return vec4(location, 1.0f);
        }
        else if (dist >= FAR_PLANE-EPSILON) {
            break;
        }

        location += ray_dir*max(MIN_STEP_SIZE, dist);
    }
    material = Material(0.0f.xxx);
    return vec4(0.0f.xxx, -1.0f);
    // return vec4(location, -1.0f);
}

#define OFFSET 0.0001f
#define SUN_DIR  normalize(vec3(-0.5f, 1.0f, 0.5f))
#define SUN_DIR2 normalize(vec3(0.2f, 1.0f, -0.2f))
#define SHADOWS 1
#define BIAS 0.01f
vec4 shade(vec3 point, vec3 ray_dir, Material material) {
    Material tmp;
    vec3 normal = vec3(
        scene_SDF(vec3(point.x+OFFSET, point.y, point.z), tmp) - scene_SDF(vec3(point.x-OFFSET, point.y, point.z), tmp),
        scene_SDF(vec3(point.x, point.y+OFFSET, point.z), tmp) - scene_SDF(vec3(point.x, point.y-OFFSET, point.z), tmp),
        scene_SDF(vec3(point.x, point.y, point.z+OFFSET), tmp) - scene_SDF(vec3(point.x, point.y, point.z-OFFSET), tmp)
    );
    normal = normalize(normal);
    vec3 diffuse = 0.0f.xxx;

    #if SHADOWS
        if (trace(point+SUN_DIR*(0.01f), SUN_DIR, tmp).w <= 0.0f) {
            diffuse += dot(normal, SUN_DIR);
        }
        // if (trace(point+SUN_DIR2*(0.01f), SUN_DIR2).w <= 0.0f) {
        //     diffuse += dot(normal, SUN_DIR2) * vec3(0.5f,1.0f,1.0f);
        // }
    #else
        diffuse += dot(normal, SUN_DIR);
        // diffuse += dot(normal, SUN_DIR2) * vec3(0.5f,1.0f,1.0f);
    #endif

    // vec3 halfway = normalize(normal + SUN_DIR);
    // float specular = pow(max(dot(normal, halfway), 0.0f), 128.0f);
    diffuse = max(diffuse, 0.1f.xxx);
    return vec4(diffuse*material.color, 1.0f);
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

    Material material;
    vec4 pos = trace(eye, ray, material);
    // if (pos.w >= 0.0f) {
    vec4 col = shade(pos.xyz, ray, material);
    // } else {
    //     col = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    // }

    imageStore(framebuffer, pix, col);
}