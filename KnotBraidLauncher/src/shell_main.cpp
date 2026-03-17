#include "ShellMainWindow.h"

#include "app/BraidingApplication.h"
#include "app/KnottingApplication.h"

#include <QApplication>
#include <QCoreApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/app_knotbraid.png")));

    LogiKnottingApp::initializeApplication();
    LogiBraidingApp::initializeApplication();

    ShellMainWindow window;
    window.setWindowIcon(app.windowIcon());

    const QStringList arguments = QCoreApplication::arguments();
    for (int i = 1; i < arguments.size(); ++i) {
        const QString argument = arguments.at(i);
        if (argument == QStringLiteral("--knotting")) {
            window.showPage(QStringLiteral("knotting"));
            break;
        }
        if (argument == QStringLiteral("--braiding")) {
            window.showPage(QStringLiteral("braiding"));
            break;
        }
        if (argument.startsWith(QStringLiteral("--page="))) {
            window.showPage(argument.mid(QStringLiteral("--page=").size()));
            break;
        }
        if (argument == QStringLiteral("--page") && i + 1 < arguments.size()) {
            window.showPage(arguments.at(i + 1));
            break;
        }
    }

    window.showMaximized();

    return app.exec();
}
