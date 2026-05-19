#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

#include "Entities/EditorExternalData.h"

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

void MainWindow::onSnapToGridChanged(bool value)
{

}

void MainWindow::on_snapToGrid_toggle_toggled(bool checked)
{
    EditorExternalData::SnapToGrid = checked;
}


void MainWindow::on_snapToGrid_select_currentIndexChanged(int index)
{
    float values[] = {
        0.1f,
        0.25f,
        0.5f,
        1.0f,
        2.0f,
        5.0f,
        10.0f,
        25.0f,
        50.0f
    };


    EditorExternalData::GridSpacing = values[index];

}


void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    EditorExternalData::editMode = (EditorExternalData::EditMode)(index+1);
}


void MainWindow::on_brushMode_currentIndexChanged(int index)
{
    EditorExternalData::SubtractiveBrush = index>0;
}


void MainWindow::on_editorMode_currentIndexChanged(int index)
{
    EditorExternalData::editMode = (EditorExternalData::EditMode)(index+1);
}

