#include "MainWindow.hpp"
#include <QPushButton>
#include <QApplication>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Raytracer");
    resize(800, 600);

    setCentralWidget(&gl_widget);

    cam_controller.set_camera(gl_widget.get_camera());

    show();

    connect(&timer, &QTimer::timeout, this, &MainWindow::main_loop);
    timer.start(16);

    mouse_captured = false;
    mouse_movement = QPoint(0,0);
}

MainWindow::~MainWindow() {

}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_Home:
            QApplication::quit();
            break;
        case Qt::Key_Escape:
            if (mouse_captured) release_mouse();
            else capture_mouse();
            break;
        case Qt::Key_F2:
            break;
        case Qt::Key_F3:
            break;
        default:
            keys_pressed.insert(event->key());
            break;
    }
    event->accept();
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    keys_pressed.erase(event->key());
    event->accept();
}

void MainWindow::mouseMoveEvent(QMouseEvent* event) {
    if (mouse_captured) {
        QPoint screen_center(geometry().left()+geometry().width()/2, geometry().top()+geometry().height()/2);
        mouse_movement += QCursor::pos()-screen_center;
        QCursor::setPos(screen_center);
    }
    event->accept();
}

void MainWindow::main_loop() {
    cam_controller.keyboard_input(keys_pressed);
    cam_controller.mouse_movement(mouse_movement.x(), mouse_movement.y());
    mouse_movement = QPoint(0,0);
    update();
    gl_widget.update();
}

void MainWindow::capture_mouse() {
    mouse_captured = true;
    QPoint screen_center(geometry().left()+geometry().width()/2, geometry().top()+geometry().height()/2);
    QCursor::setPos(screen_center);
    mouse_movement = QPoint(0,0);
    grabMouse(Qt::BlankCursor);
    setMouseTracking(true);
}

void MainWindow::release_mouse() {
    mouse_captured = false;
    releaseMouse();
    setMouseTracking(false);
}