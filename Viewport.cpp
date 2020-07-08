#include "Viewport.hpp"

#include <QApplication>
#include <QKeyEvent>
#include <QGridLayout>
#include <QDebug>

Viewport::Viewport(QWidget* parent) : QWidget(parent), gl_widget(this) {
    QGridLayout* layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(&gl_widget, 0, 0);
    setFocusPolicy(Qt::StrongFocus);

    renderer = gl_widget.get_renderer();
    cam_controller.set_camera(renderer->get_camera());

    mouse_captured = false;
}

Viewport::~Viewport() {}

void Viewport::keyPressEvent(QKeyEvent* event) {
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
            cam_controller.key_event(event);
            break;
    }
    event->accept();
}

void Viewport::keyReleaseEvent(QKeyEvent *event) {
    cam_controller.key_event(event);
    event->accept();
}

void Viewport::mouseMoveEvent(QMouseEvent* event) {
    if (mouse_captured) {
        QPoint screen_center(geometry().left()+geometry().width()/2, geometry().top()+geometry().height()/2);
        QPoint mouse_movement = QCursor::pos()-screen_center;
        cam_controller.mouse_moved(mouse_movement.x(), mouse_movement.y());
        QCursor::setPos(screen_center);
    }
    event->accept();
}

void Viewport::main_loop() {
    cam_controller.main_loop();
    gl_widget.update();
}

void Viewport::capture_mouse() {
    mouse_captured = true;
    QPoint screen_center(geometry().left()+geometry().width()/2, geometry().top()+geometry().height()/2);
    QCursor::setPos(screen_center);
    grabMouse(Qt::BlankCursor);
    setMouseTracking(true);
}

void Viewport::release_mouse() {
    mouse_captured = false;
    releaseMouse();
    setMouseTracking(false);
}