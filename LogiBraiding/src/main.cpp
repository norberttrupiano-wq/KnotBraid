#include "app/BraidingApplication.h"
#include "ui/BraidingMainWindow.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(QStringLiteral(":/icons/app_logibraiding.png")));
    LogiBraidingApp::initializeApplication();

    LogiBraidingApp::MainWindow w;
    w.setWindowIcon(a.windowIcon());
    w.showMaximized();
    return a.exec();
}
