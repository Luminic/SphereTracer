#version 450 core

layout (binding = 0) uniform sampler2D equirectangular_map;
layout (binding = 1, rgba32f) uniform imageCube cube_map;

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

#define PI 3.1415926536f
#define DOUBLE_PI 6.2831853072f

vec2 cube_map_to_equirectangular_uvs(vec3 tex_coords) {
    vec2 phi_theta = vec2(atan(tex_coords.z, tex_coords.x), asin(tex_coords.y));
    phi_theta.x = phi_theta.x / DOUBLE_PI;
    phi_theta.y = phi_theta.y / PI;
    phi_theta += 0.5f;
    return phi_theta;
}

void main() {
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
    int face = int(gl_GlobalInvocationID.z);

    ivec2 size = imageSize(cube_map);
    if (pix.x >= size.x || pix.y >= size.y || face >= 6) {
        return;
    }

    // Note: normalized_pix.y increases downwards
    vec2 normalized_pix = vec2(pix)/size.x*2.0f - 1.0f;
    vec3 loc;
    switch (face) {
    case 0: // GL_TEXTURE_CUBE_MAP_POSITIVE_X (Right)
        loc = vec3(-1.0f, -normalized_pix.y, normalized_pix.x);
        break;
    case 1: // GL_TEXTURE_CUBE_MAP_NEGATIVE_X (Left)
        loc = vec3(1.0f, -normalized_pix.y, -normalized_pix.x);
        break;
    case 2: // GL_TEXTURE_CUBE_MAP_POSITIVE_Y (Top)
        loc = vec3(-normalized_pix.x, 1.0f, -normalized_pix.y);
        break;
    case 3: // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y (Bottom)
        loc = vec3(-normalized_pix.x, -1.0f, normalized_pix.y);
        break;
    case 4: // GL_TEXTURE_CUBE_MAP_POSITIVE_Z (Back)
        loc = vec3(-normalized_pix.x, -normalized_pix.y, -1.0f);
        break;
    case 5: // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z (Front)
        loc = vec3(normalized_pix.x, -normalized_pix.y, 1.0f);
        break;
    }

    vec2 tex_coords = cube_map_to_equirectangular_uvs(normalize(loc));
    vec4 col = texture(equirectangular_map, tex_coords);

    imageStore(cube_map, ivec3(pix, face), col);
}