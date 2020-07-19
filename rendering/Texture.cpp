#include "Texture.hpp"

#include "Shader.hpp"

#include <QImage>
#include <QFile>
#include <QDebug>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>


uint32_t round_up_to_pow_2(uint32_t x);

Texture::Texture(QObject* parent) : QObject(parent) {}

Texture::~Texture() {}

void Texture::load(const char* path) {
    QImage img = QImage(path).convertToFormat(QImage::Format_RGBA8888).mirrored(false, true);
    load(img);
}

void Texture::load(QImage img) {
    initializeOpenGLFunctions();

    glGenTextures(1, &id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);

    set_params();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
}

void Texture::create(unsigned int width, unsigned int height) {
    initializeOpenGLFunctions();

    glGenTextures(1, &id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);

    set_params();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)0);
}

void Texture::load_cube_map(const char* equirectangular_path, unsigned int size) {
    initializeOpenGLFunctions();

    stbi_set_flip_vertically_on_load(true);
    int width, height, nr_channels;
    float* data = stbi_loadf(equirectangular_path, &width, &height, &nr_channels, 0);

    unsigned int equirectangular_map;
    glGenTextures(1, &equirectangular_map);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, equirectangular_map);

    set_params(equirectangular_map);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGB, GL_FLOAT, data);

    stbi_image_free(data);

    // Convert equirectangular-map into a cubemap
    glGenTextures(1, &id);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    for (int i=0; i<6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGBA32F, size, size, 0, GL_RGBA, GL_FLOAT, nullptr);
    }
    glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    Shader eq_to_cubemap;
    ShaderStage eq_to_cubemap_compute{GL_COMPUTE_SHADER, "rendering/shaders/equirectangular_to_cube_map.glsl"};

    eq_to_cubemap.load_shaders(&eq_to_cubemap_compute, 1);
    eq_to_cubemap.validate();

    glUseProgram(eq_to_cubemap.get_id());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, equirectangular_map);

    glActiveTexture(GL_TEXTURE1);
    glBindImageTexture(1, id, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    unsigned int worksize = round_up_to_pow_2(size);
    int work_group_size[3];
    glGetProgramiv(eq_to_cubemap.get_id(), GL_COMPUTE_WORK_GROUP_SIZE, work_group_size);
    glDispatchCompute(worksize/work_group_size[0], worksize/work_group_size[1], 8/work_group_size[2]);

    glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glDeleteTextures(1, &equirectangular_map);

    glActiveTexture(GL_TEXTURE0);
}


void Texture::set_params(unsigned int tex_id) {
    if (tex_id == 0) {
        tex_id = id;
    }

    glBindTexture(GL_TEXTURE_2D, tex_id);

    glTextureParameteri(tex_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(tex_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTextureParameteri(tex_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(tex_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Texture::resize(unsigned int width, unsigned int height) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)0);
}

unsigned int Texture::get_id() {
    return id;
}
