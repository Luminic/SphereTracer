#include "MainWindow.hpp"
#include <QPushButton>
#include <QApplication>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Sphere Marcher");
    // resize(800, 600);
    resize(400, 300);

    setCentralWidget(&viewport);

    show();

    connect(&timer, &QTimer::timeout, this, &MainWindow::main_loop);
    timer.start(16);
}

MainWindow::~MainWindow() {}

void MainWindow::main_loop() {
    viewport.main_loop();
}