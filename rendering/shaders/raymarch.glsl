#version 450 core

layout (binding = 0, rgba32f) uniform image2D framebuffer;

uniform vec3 eye;
uniform vec3 ray00;
uniform vec3 ray10;
uniform vec3 ray01;
uniform vec3 ray11;

uniform int time;

#define PI 3.1415926536f

#define EPSILON 0.000001f
#define NEAR_PLANE 0.1f
#define FAR_PLANE 100.0f
#define MIN_STEP_SIZE 0.0001f
#define MAX_STEPS 500

struct Material {
    vec3 albedo;
    vec3 F0;
    float roughness;
    float metalness;
    float AO;
};
#define DEFAULT_MATERIAL Material(vec3(1.0f), vec3(0.05f), 0.5f, 0.0f, 0.5f)
#define MATERIAL(x) Material(x, vec3(0.05f), 0.3f, 0.0f, 0.5f)


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
    material = MATERIAL(pow(trap.xyz, 3.0f.xxx));
    // Bounding sphere
    if (length(point) >= 2.0f) {
        return length(point)-1.5f;
    }
    return 0.25*log(dot(z,z))*length(z)/dr;
    // View cross sections of the colors embedded in a sphere:
    // return max(min(0.25*log(dot(z,z))*length(z)/dr, length(point)-1.5f), point.y+1.5f*(abs((time%10000)/2500.0f-2.0f)-1.0f));
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
            material = MATERIAL(vec3(1.0f,0.0f,0.0f));
        }
    }

    d = cylinder_SDF(point, Cylinder(vec3(-3.0f, 0.0f, 0.0f), 0.5f, 1.5));
    if (d <= dist) {
        dist = d;
        material = MATERIAL(1.0f.xxx);
    }

    // Floor plane
    d = max(point.y+1.5f, -1.6f-point.y);
    if (d <= dist) {
        dist = d;
        material = MATERIAL(1.0f.xxx);
    }
    return dist;
}

vec4 camera_ray(vec3 ray_origin, vec3 ray_dir, out Material material) {
    float dist_traveled = 0.0f;
    for (int i=0; i<MAX_STEPS; i++) {
        float dist = scene_SDF(ray_origin+ray_dir*dist_traveled, material);

        if (dist <= 0.0f) {
            return vec4(ray_origin+ray_dir*dist_traveled, 1.0f);
        }
        else if (dist >= FAR_PLANE-EPSILON || dist_traveled >= FAR_PLANE-EPSILON) {
            break;
        }
        dist_traveled += max(MIN_STEP_SIZE, dist);
    }
    material = MATERIAL(0.0f.xxx);
    return vec4(0.0f.xxx, -1.0f);
}

#define BIAS 0.01f
float shadow_ray(vec3 ray_origin, vec3 ray_dir, float max_dist, float sharpness) {
    float dist_traveled = BIAS;
    float shadowing = 1.0f;
    for (int i=0; i<MAX_STEPS; i++) {
        Material tmp;
        float dist = scene_SDF(ray_origin+ray_dir*dist_traveled, tmp);

        if (dist <= 0.0f) {
            return 0.0f;
        }
        if (dist >= FAR_PLANE-EPSILON || dist_traveled >= max_dist-EPSILON) {
            break;
        }


        shadowing = min(shadowing, sharpness*dist/dist_traveled);

        // // Better smooth shadows but doesn't work well with distance estimators:
        // float d = dist * dist / 4.0f;
        // d = (prev_dist * prev_dist - d) * d;
        // d = 2*sqrt(d) / prev_dist;

        // float n = sqrt(dist * dist - d * d);

        // shadowing = min(shadowing, sharpness*dist/max(0.0f, dist_traveled-n));

        // prev_dist = dist;

        dist_traveled += max(MIN_STEP_SIZE, dist);
    }
    return shadowing;
}


float NDF_trowbridge_reitz_GGX(vec3 view, vec3 normal, vec3 light, float alpha) {
    vec3 halfway = normalize(view + light);
    float a2 = alpha * alpha;

    return a2 / ( PI * pow( pow(max(dot(normal, halfway), 0.0f), 2) * (a2 - 1) + 1, 2) );
}

float GF_schlick_GGX(float n_dot_v, float alpha) {
    // Schlick approximation
    float k = alpha / 2.0f;

    return n_dot_v / ( n_dot_v * (1-k) + k );
}

float GF_smith(vec3 view, vec3 normal, vec3 light, float alpha) {
    float n_dot_v = max(dot(normal, view), 0.0f);
    float n_dot_l = max(dot(normal, light), 0.0f);
    
    return GF_schlick_GGX(n_dot_v, alpha) * GF_schlick_GGX(n_dot_l, alpha);
}

vec3 F_schlick(vec3 view, vec3 normal, vec3 F0) {
    // F0 is the reflectivity at normal incidence
    return F0 + (1.0f - F0) * pow((1.0f - dot(view, normal)), 5);
}

vec3 cook_torrance_BRDF(vec3 view, vec3 normal, vec3 light, Material material) {
    vec3 lambertian_diffuse = material.albedo / PI;

    float alpha = material.roughness * material.roughness;
    vec3 F0 = material.F0;
    F0 = mix(F0, material.albedo, material.metalness);

    float NDF = NDF_trowbridge_reitz_GGX(view, normal, light, alpha);
    float GF = GF_smith(view, normal, light, alpha);
    vec3 F = F_schlick(view, normal, F0);

    vec3 kD = (1.0f.xxx - F) * (1.0f - material.metalness);

    vec3 numer = NDF * GF * F;
    float denom = 4.0f * max(dot(normal, view), 0.0f) * max(dot(normal, light), 0.0f);

    return kD*lambertian_diffuse + numer/max(denom, 0.001f);
}

#define OFFSET 0.0001f
#define SUN_DIR  normalize(vec3(-0.5f, 1.0f, 0.5f))
#define SHADOWS 1
#define AMBIENT_MULTIPLIER 0.03
vec4 shade(vec3 point, vec3 ray_dir, Material material) {
    Material tmp;
    bool tmp2;
    vec3 normal = vec3(
        scene_SDF(vec3(point.x+OFFSET, point.y, point.z), tmp) - scene_SDF(vec3(point.x-OFFSET, point.y, point.z), tmp),
        scene_SDF(vec3(point.x, point.y+OFFSET, point.z), tmp) - scene_SDF(vec3(point.x, point.y-OFFSET, point.z), tmp),
        scene_SDF(vec3(point.x, point.y, point.z+OFFSET), tmp) - scene_SDF(vec3(point.x, point.y, point.z-OFFSET), tmp)
    );
    normal = normalize(normal);
    vec3 diffuse = 0.0f.xxx;

    vec3 radiance = vec3(1.0f);
    #if SHADOWS
        radiance *= shadow_ray(point, SUN_DIR, FAR_PLANE, 32);
    #endif

    vec3 color = cook_torrance_BRDF(-ray_dir, normal, SUN_DIR, material);
    color *= radiance * max(dot(normal, SUN_DIR), 0.0f);
    
    color += material.albedo * material.AO * AMBIENT_MULTIPLIER;
    return vec4(color, 1.0f);
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
    vec4 pos = camera_ray(eye, ray, material);

    vec4 col;
    if (pos.w >= 0) {
        col = shade(pos.xyz, ray, material);
    } else {
        col = vec4(0.0f.xxx, 1.0f);
    }

    imageStore(framebuffer, pix, col);
}