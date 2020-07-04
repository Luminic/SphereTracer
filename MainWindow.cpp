#include "MainWindow.hpp"
#include <QPushButton>
#include <QApplication>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Raytracer");
    resize(800, 600);

    setCentralWidget(&viewport);

    show();

    connect(&timer, &QTimer::timeout, this, &MainWindow::main_loop);
    timer.start(16);
}

MainWindow::~MainWindow() {}

void MainWindow::main_loop() {
    viewport.main_loop();
}