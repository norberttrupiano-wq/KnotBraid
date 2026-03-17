#ifndef KNOTBRAID_MAINWINDOW_H
#define KNOTBRAID_MAINWINDOW_H

#include <QMainWindow>

class QCheckBox;
class QLabel;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void launchLogiKnotting();
    void launchLogiBraiding();
    void launchBoth();
    void buildBoth();

private:
    struct AppTarget {
        QString displayName;
        QString projectDir;
        QString exeName;
    };

    QCheckBox *m_buildIfMissingCheck = nullptr;
    QLabel *m_statusLabel = nullptr;

    QString findRepoRoot() const;
    QString findExecutable(const AppTarget &target) const;
    QString qtPrefixPath() const;
    QString qtToolPath(const QString &toolName) const;
    bool qtRuntimeNeedsDeployment(const QString &exePath) const;

    bool runCommand(const QString &program,
                    const QStringList &arguments,
                    const QString &workingDirectory,
                    QString *output,
                    QString *errorMessage);
    bool deployQtRuntime(const QString &exePath, QString *errorMessage);
    bool buildTarget(const AppTarget &target, QString *errorMessage);
    bool launchTarget(const AppTarget &target, QString *errorMessage);
    void setStatus(const QString &message, bool isError = false);
};

#endif // KNOTBRAID_MAINWINDOW_H
