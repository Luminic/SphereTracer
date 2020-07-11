#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <QObject>
#include <QOpenGLFunctions_4_5_Core>

#include "Camera.hpp"
#include "Shader.hpp"
#include "Texture.hpp"


class Renderer : public QObject, protected QOpenGLFunctions_4_5_Core {
    Q_OBJECT;

public:
    Renderer(QObject* parent=nullptr);
    Texture* initialize(unsigned int width, unsigned int height);
    void resize(unsigned int width, unsigned int height);

    Texture* render(int time);

    Camera* get_camera();

private:
    Camera camera;

    Shader render_shader;
    int work_group_size[3];
    Texture render_result;

    unsigned int width;
    unsigned int height;
};

#endif