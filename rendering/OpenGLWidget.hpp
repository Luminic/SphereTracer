#ifndef OPENGL_WIDGET_HPP
#define OPENGL_WIDGET_HPP

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Core>

#include "Camera.hpp"
#include "Shader.hpp"

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

    unsigned int frame_vbo;
    unsigned int frame_vao;
    ShaderProgram frame_shader;
};

#endif