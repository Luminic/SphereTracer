#ifndef VIEWPORT_HPP
#define VIEWPORT_HPP

#include <QWidget>
#include <QElapsedTimer>

#include "rendering/OpenGLWidget.hpp"
#include "rendering/Renderer.hpp"
#include "CameraController.hpp"

class Viewport : public QWidget {
    Q_OBJECT;

public:
    Viewport(QWidget* parent=nullptr);
    ~Viewport();

    void main_loop();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    void capture_mouse();
    void release_mouse();

    void screenshot();

    OpenGLWidget gl_widget;
    Renderer* renderer;

    CameraController cam_controller;
    bool mouse_captured;

    QElapsedTimer timer;
};

#endif