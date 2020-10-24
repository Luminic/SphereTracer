#include "Viewport.hpp"

#include <QApplication>
#include <QKeyEvent>
#include <QGridLayout>
#include <QFile>
#include <QFileDialog>
#include <QDebug>

Viewport::Viewport(QWidget* parent) : QWidget(parent), gl_widget(this) {
    QGridLayout* layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(&gl_widget, 0, 0);
    setFocusPolicy(Qt::StrongFocus);

    renderer = gl_widget.get_renderer();
    cam_controller.set_camera(renderer->get_camera());

    mouse_captured = false;
    timer.start();
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
            screenshot();
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
        QPoint screen_center = mapToGlobal(QPoint(geometry().left()+geometry().width()/2, geometry().top()+geometry().height()/2));
        QPoint mouse_movement = QCursor::pos()-screen_center;
        cam_controller.mouse_moved(mouse_movement.x(), mouse_movement.y());
        QCursor::setPos(screen_center);
    }
    event->accept();
}

void Viewport::main_loop() {
    cam_controller.main_loop();
    gl_widget.main_loop(timer.elapsed());
}

void Viewport::capture_mouse() {
    mouse_captured = true;
    QPoint screen_center = mapToGlobal(QPoint(geometry().left()+geometry().width()/2, geometry().top()+geometry().height()/2));
    QCursor::setPos(screen_center);
    grabMouse(Qt::BlankCursor);
    setMouseTracking(true);
}

void Viewport::release_mouse() {
    mouse_captured = false;
    releaseMouse();
    setMouseTracking(false);
}

void Viewport::screenshot() {
    if (mouse_captured)
        release_mouse();
    QFileDialog dialog(this);
    dialog.setWindowTitle("Save Screenshot");
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter(tr("Images (*.png *.bmp *.jpg)"));
    if (dialog.exec()) {
        QStringList fileNames(dialog.selectedFiles());
        if (QRegExp(".+\\.(png|bmp|jpg)").exactMatch(fileNames.at(0))) {
            gl_widget.grabFramebuffer().save(fileNames.at(0));
        } else {
            qDebug() << "Save error: bad format or filename.";
        }
    }
}