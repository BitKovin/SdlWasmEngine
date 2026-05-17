#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // All layout comes from mainwindow.ui — edit it in Qt Designer.
    // The EngineViewportWidget placed there via promotion handles
    // attach/detach automatically through its show/hide events.
}

MainWindow::~MainWindow()
{
    delete ui;
}
