#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void onSnapToGridChanged(bool value);

private slots:
    void on_snapToGrid_toggle_toggled(bool checked);

    void on_snapToGrid_select_currentIndexChanged(int index);

    void on_comboBox_currentIndexChanged(int index);

    void on_brushMode_currentIndexChanged(int index);

    void on_editorMode_currentIndexChanged(int index);

private:
    Ui::MainWindow* ui;
};
