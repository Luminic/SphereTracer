#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <QMainWindow>

#include "rendering/OpenGLWidget.hpp"
#include "CameraController.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT;

public:
    MainWindow(QWidget* parent=nullptr);
    ~MainWindow();

private:
    OpenGLWidget* gl_widget;
    CameraController cam_controller;
};

#endif
