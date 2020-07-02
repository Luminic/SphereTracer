#include "MainWindow.hpp"
#include <QPushButton>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Raytracer");
    resize(800, 600);

    gl_widget = new OpenGLWidget(this);
    setCentralWidget(gl_widget);

    cam_controller.set_camera(gl_widget->get_camera());

    show();
}

MainWindow::~MainWindow() {

}