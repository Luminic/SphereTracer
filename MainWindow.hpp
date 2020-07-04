#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <QMainWindow>
#include <QTimer>

#include "rendering/OpenGLWidget.hpp"
#include "CameraController.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT;

public:
    MainWindow(QWidget* parent=nullptr);
    ~MainWindow();


protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    void main_loop();

    void capture_mouse();
    void release_mouse();

    QTimer timer;

    OpenGLWidget gl_widget;
    CameraController cam_controller;

    std::unordered_set<int> keys_pressed;
    bool mouse_captured;
    QPoint mouse_movement;
};

#endif
