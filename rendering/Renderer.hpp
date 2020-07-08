#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <QObject>
#include <QOpenGLFunctions_4_5_Core>

#include "Camera.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Vertex.hpp"


class Renderer : public QObject, protected QOpenGLFunctions_4_5_Core {
    Q_OBJECT;

public:
    Renderer(QObject* parent=nullptr);
    void initialize(unsigned int width, unsigned int height);
    void resize(unsigned int width, unsigned int height);

    Texture* render();

    Camera* get_camera();

private:
    Camera camera;

    Shader render_shader;
    int work_group_size[3];
    Texture render_result;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int vertex_ssbo;
    unsigned int index_ssbo;

    unsigned int width;
    unsigned int height;
};

#endif