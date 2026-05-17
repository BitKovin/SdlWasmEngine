#include "mainwindow.h"
#include "EngineInstance.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Initialize the engine once before any window is shown.
    // It will keep running for the entire app lifetime.
    EngineInstance::get().initialize();

    MainWindow w;
    w.show();

    return app.exec();
}
