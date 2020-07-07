#ifndef OPENGL_WIDGET_HPP
#define OPENGL_WIDGET_HPP

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Core>

#include <vector>

#include "Camera.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Vertex.hpp"

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_5_Core {
    Q_OBJECT;

public:
    OpenGLWidget(QWidget* parent=nullptr);
    ~OpenGLWidget();

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    Camera* get_camera();

private:
    Camera camera;

    Shader render_shader;
    int work_group_size[3];
    Texture render_result;

    unsigned int frame_vbo;
    unsigned int frame_vao;
    Shader frame_shader;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int vertex_ssbo;
};

#endif