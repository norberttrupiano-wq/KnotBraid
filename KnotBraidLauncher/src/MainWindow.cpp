#include "MainWindow.h"

#include <QApplication>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QProcess>
#include <QPushButton>
#include <QStringList>
#include <QVersionNumber>
#include <QVBoxLayout>
#include <QWidget>

#ifdef Q_OS_WIN
#include <windows.h>
#include <winver.h>
#endif

namespace {

QString tail(const QString &text, int maxChars)
{
    if (text.size() <= maxChars) {
        return text;
    }
    return text.right(maxChars);
}

QString defaultQtPrefixPath()
{
    const QDir qtRoot(QStringLiteral("C:/Qt"));
    if (qtRoot.exists()) {
        QVersionNumber bestVersion;
        QString bestPath;

        const QFileInfoList entries = qtRoot.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot,
                                                           QDir::Name);

        for (const QFileInfo &entry : entries) {
            int suffixIndex = -1;
            const QVersionNumber version = QVersionNumber::fromString(entry.fileName(), &suffixIndex);
            if (suffixIndex != entry.fileName().size() || version.majorVersion() < 6) {
                continue;
            }

            const QString candidate = QDir(entry.absoluteFilePath())
                                          .filePath(QStringLiteral("msvc2022_64"));
            if (!QDir(candidate).exists()) {
                continue;
            }

            if (bestPath.isEmpty() || QVersionNumber::compare(version, bestVersion) > 0) {
                bestVersion = version;
                bestPath = candidate;
            }
        }

        if (!bestPath.isEmpty()) {
            return QDir::toNativeSeparators(bestPath);
        }
    }

    return QStringLiteral("C:/Qt/6.10.2/msvc2022_64");
}

#ifdef Q_OS_WIN
QString windowsFileVersion(const QString &path)
{
    const std::wstring nativePath = QDir::toNativeSeparators(path).toStdWString();
    DWORD unused = 0;
    const DWORD versionInfoSize = GetFileVersionInfoSizeW(nativePath.c_str(), &unused);
    if (versionInfoSize == 0) {
        return QString();
    }

    QByteArray buffer(static_cast<int>(versionInfoSize), Qt::Uninitialized);
    if (!GetFileVersionInfoW(nativePath.c_str(), 0, versionInfoSize, buffer.data())) {
        return QString();
    }

    VS_FIXEDFILEINFO *fileInfo = nullptr;
    UINT fileInfoSize = 0;
    if (!VerQueryValueW(buffer.data(),
                        L"\\",
                        reinterpret_cast<LPVOID *>(&fileInfo),
                        &fileInfoSize) ||
        fileInfo == nullptr) {
        return QString();
    }

    return QStringLiteral("%1.%2.%3.%4")
        .arg(HIWORD(fileInfo->dwFileVersionMS))
        .arg(LOWORD(fileInfo->dwFileVersionMS))
        .arg(HIWORD(fileInfo->dwFileVersionLS))
        .arg(LOWORD(fileInfo->dwFileVersionLS));
}
#endif

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("KnotBraidLauncher"));
    resize(680, 300);

    auto *central = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(16, 16, 16, 16);
    rootLayout->setSpacing(12);

    auto *title = new QLabel(QStringLiteral("KnotBraidLauncher - Outils developpeur"), central);
    QFont titleFont = title->font();
    titleFont.setPointSize(titleFont.pointSize() + 3);
    titleFont.setBold(true);
    title->setFont(titleFont);

    auto *subtitle = new QLabel(
        QStringLiteral("Compilation, redeploiement Qt et lancement des modules LogiKnotting et LogiBraiding."),
        central);
    subtitle->setWordWrap(true);

    auto *buttonsRow = new QHBoxLayout();
    buttonsRow->setSpacing(10);

    auto *knottingButton = new QPushButton(QStringLiteral("Ouvrir LogiKnotting"), central);
    auto *braidingButton = new QPushButton(QStringLiteral("Ouvrir LogiBraiding"), central);
    auto *bothButton = new QPushButton(QStringLiteral("Ouvrir les deux"), central);
    auto *buildButton = new QPushButton(QStringLiteral("Compiler les deux"), central);

    knottingButton->setMinimumHeight(42);
    braidingButton->setMinimumHeight(42);
    bothButton->setMinimumHeight(42);
    buildButton->setMinimumHeight(42);

    buttonsRow->addWidget(knottingButton);
    buttonsRow->addWidget(braidingButton);
    buttonsRow->addWidget(bothButton);
    buttonsRow->addWidget(buildButton);

    m_buildIfMissingCheck = new QCheckBox(
        QStringLiteral("Compiler automatiquement si executable absent"),
        central);
    m_buildIfMissingCheck->setChecked(true);

    auto *qtHint = new QLabel(
        QStringLiteral("Qt prefix: variable KNOTBRAID_QT_PREFIX, sinon auto-detection de C:/Qt/*/msvc2022_64."),
        central);
    qtHint->setWordWrap(true);

    m_statusLabel = new QLabel(central);
    m_statusLabel->setWordWrap(true);

    rootLayout->addWidget(title);
    rootLayout->addWidget(subtitle);
    rootLayout->addLayout(buttonsRow);
    rootLayout->addWidget(m_buildIfMissingCheck);
    rootLayout->addWidget(qtHint);
    rootLayout->addWidget(m_statusLabel);
    rootLayout->addStretch(1);

    setCentralWidget(central);

    connect(knottingButton, &QPushButton::clicked, this, &MainWindow::launchLogiKnotting);
    connect(braidingButton, &QPushButton::clicked, this, &MainWindow::launchLogiBraiding);
    connect(bothButton, &QPushButton::clicked, this, &MainWindow::launchBoth);
    connect(buildButton, &QPushButton::clicked, this, &MainWindow::buildBoth);

    setStatus(QStringLiteral("Pret. Choisissez une action."));
}

QString MainWindow::findRepoRoot() const
{
    QDir dir(QCoreApplication::applicationDirPath());

    for (int i = 0; i < 8; ++i) {
        if (dir.exists(QStringLiteral("LogiKnotting")) && dir.exists(QStringLiteral("LogiBraiding"))) {
            return dir.absolutePath();
        }

        if (!dir.cdUp()) {
            break;
        }
    }

    return QDir::currentPath();
}

QString MainWindow::findExecutable(const AppTarget &target) const
{
    const QString root = findRepoRoot();

    const QStringList candidates = {
        QDir(root).filePath(target.projectDir + QStringLiteral("/build/Release/") + target.exeName),
        QDir(root).filePath(target.projectDir + QStringLiteral("/build/Debug/") + target.exeName),
        QDir(root).filePath(target.projectDir + QStringLiteral("/build/") + target.exeName)
    };

    for (const QString &path : candidates) {
        QFileInfo info(path);
        if (info.exists() && info.isFile()) {
            return info.absoluteFilePath();
        }
    }

    return QString();
}

QString MainWindow::qtPrefixPath() const
{
    const QString envValue = qEnvironmentVariable("KNOTBRAID_QT_PREFIX").trimmed();
    if (!envValue.isEmpty()) {
        return envValue;
    }

    return defaultQtPrefixPath();
}

QString MainWindow::qtToolPath(const QString &toolName) const
{
    const QString candidate = QDir(qtPrefixPath()).filePath(QStringLiteral("bin/") + toolName);
    QFileInfo info(candidate);
    if (info.exists() && info.isFile()) {
        return info.absoluteFilePath();
    }

    return QString();
}

bool MainWindow::qtRuntimeNeedsDeployment(const QString &exePath) const
{
#ifndef Q_OS_WIN
    Q_UNUSED(exePath);
    return false;
#else
    const QStringList runtimeDlls = {
        QStringLiteral("Qt6Core.dll"),
        QStringLiteral("Qt6Gui.dll"),
        QStringLiteral("Qt6Widgets.dll")
    };

    const QDir exeDir(QFileInfo(exePath).absolutePath());
    const QDir qtBinDir(QDir(qtPrefixPath()).filePath(QStringLiteral("bin")));

    for (const QString &dllName : runtimeDlls) {
        const QString deployedPath = exeDir.filePath(dllName);
        const QString referencePath = qtBinDir.filePath(dllName);

        QFileInfo referenceInfo(referencePath);
        if (!referenceInfo.exists() || !referenceInfo.isFile()) {
            continue;
        }

        QFileInfo deployedInfo(deployedPath);
        if (!deployedInfo.exists() || !deployedInfo.isFile()) {
            return true;
        }

        if (windowsFileVersion(deployedPath) != windowsFileVersion(referencePath)) {
            return true;
        }
    }

    return false;
#endif
}

bool MainWindow::runCommand(const QString &program,
                            const QStringList &arguments,
                            const QString &workingDirectory,
                            QString *output,
                            QString *errorMessage)
{
    QProcess process;
    process.setProgram(program);
    process.setArguments(arguments);
    if (!workingDirectory.isEmpty()) {
        process.setWorkingDirectory(workingDirectory);
    }
    process.setProcessChannelMode(QProcess::MergedChannels);

    process.start();
    if (!process.waitForStarted(10000)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Impossible de demarrer '%1': %2")
                                .arg(program, process.errorString());
        }
        return false;
    }

    if (!process.waitForFinished(-1)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Commande interrompue: %1")
                                .arg(program);
        }
        return false;
    }

    const QString commandOutput = QString::fromLocal8Bit(process.readAllStandardOutput());
    if (output) {
        *output = commandOutput;
    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Commande en echec: %1 %2\n%3")
                                .arg(program,
                                     arguments.join(QLatin1Char(' ')),
                                     tail(commandOutput, 1200));
        }
        return false;
    }

    if (errorMessage) {
        errorMessage->clear();
    }
    return true;
}

bool MainWindow::deployQtRuntime(const QString &exePath, QString *errorMessage)
{
    const QString windeployqtPath = qtToolPath(QStringLiteral("windeployqt.exe"));
    if (windeployqtPath.isEmpty()) {
        if (errorMessage) {
            errorMessage->clear();
        }
        return true;
    }

    const QFileInfo exeInfo(exePath);
    if (!exeInfo.exists() || !exeInfo.isFile()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Executable introuvable pour le deploiement Qt: %1")
                                .arg(exePath);
        }
        return false;
    }

    QString output;
    if (!runCommand(windeployqtPath,
                    {QStringLiteral("--release"),
                     QStringLiteral("--force"),
                     QStringLiteral("--compiler-runtime"),
                     QDir::toNativeSeparators(exeInfo.absoluteFilePath())},
                    exeInfo.absolutePath(),
                    &output,
                    errorMessage)) {
        return false;
    }

    if (errorMessage) {
        errorMessage->clear();
    }
    return true;
}

bool MainWindow::buildTarget(const AppTarget &target, QString *errorMessage)
{
    const QString root = findRepoRoot();
    const QString projectPath = QDir(root).filePath(target.projectDir);
    const QString buildPath = QDir(projectPath).filePath(QStringLiteral("build"));

    if (!QDir(projectPath).exists()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Dossier projet introuvable: %1").arg(projectPath);
        }
        return false;
    }

    QString output;

    setStatus(QStringLiteral("Configuration CMake de %1...").arg(target.displayName));
    qApp->processEvents();

    if (!runCommand(QStringLiteral("cmake"),
                    {QStringLiteral("-S"), projectPath,
                     QStringLiteral("-B"), buildPath,
                     QStringLiteral("-DCMAKE_PREFIX_PATH=%1").arg(qtPrefixPath())},
                    root,
                    &output,
                    errorMessage)) {
        return false;
    }

    setStatus(QStringLiteral("Compilation de %1 (Release)...").arg(target.displayName));
    qApp->processEvents();

    if (!runCommand(QStringLiteral("cmake"),
                    {QStringLiteral("--build"), buildPath,
                     QStringLiteral("--config"), QStringLiteral("Release")},
                    root,
                    &output,
                    errorMessage)) {
        return false;
    }

    const QString exePath = findExecutable(target);
    if (exePath.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Executable introuvable apres compilation pour %1.")
                                .arg(target.displayName);
        }
        return false;
    }

    setStatus(QStringLiteral("Deploiement Qt de %1...").arg(target.displayName));
    qApp->processEvents();

    if (!deployQtRuntime(exePath, errorMessage)) {
        return false;
    }

    if (errorMessage) {
        errorMessage->clear();
    }
    return true;
}

bool MainWindow::launchTarget(const AppTarget &target, QString *errorMessage)
{
    QString exePath = findExecutable(target);

    if (exePath.isEmpty() && m_buildIfMissingCheck && m_buildIfMissingCheck->isChecked()) {
        QString buildError;
        if (!buildTarget(target, &buildError)) {
            if (errorMessage) {
                *errorMessage = buildError;
            }
            return false;
        }
        exePath = findExecutable(target);
    }

    if (exePath.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Executable introuvable pour %1. Compilez d'abord %2.")
                                .arg(target.displayName, target.projectDir);
        }
        return false;
    }

    if (qtRuntimeNeedsDeployment(exePath)) {
        setStatus(QStringLiteral("Preparation du runtime Qt pour %1...").arg(target.displayName));
        qApp->processEvents();

        if (!deployQtRuntime(exePath, errorMessage)) {
            return false;
        }
    }

    const QString workingDir = QFileInfo(exePath).absolutePath();
    const bool started = QProcess::startDetached(exePath, QStringList(), workingDir);

    if (!started) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Echec du lancement de %1 depuis %2")
                                .arg(target.displayName, exePath);
        }
        return false;
    }

    if (errorMessage) {
        errorMessage->clear();
    }
    return true;
}

void MainWindow::setStatus(const QString &message, bool isError)
{
    if (!m_statusLabel) {
        return;
    }

    if (isError) {
        m_statusLabel->setStyleSheet(QStringLiteral("color: #A40000;"));
    } else {
        m_statusLabel->setStyleSheet(QString());
    }
    m_statusLabel->setText(message);
}

void MainWindow::launchLogiKnotting()
{
    const AppTarget target{QStringLiteral("LogiKnotting"),
                           QStringLiteral("LogiKnotting"),
                           QStringLiteral("LogiKnotting.exe")};

    QString error;
    if (launchTarget(target, &error)) {
        setStatus(QStringLiteral("LogiKnotting lance."));
        return;
    }

    setStatus(error, true);
}

void MainWindow::launchLogiBraiding()
{
    const AppTarget target{QStringLiteral("LogiBraiding"),
                           QStringLiteral("LogiBraiding"),
                           QStringLiteral("LogiBraiding.exe")};

    QString error;
    if (launchTarget(target, &error)) {
        setStatus(QStringLiteral("LogiBraiding lance."));
        return;
    }

    setStatus(error, true);
}

void MainWindow::launchBoth()
{
    const AppTarget knottingTarget{QStringLiteral("LogiKnotting"),
                                   QStringLiteral("LogiKnotting"),
                                   QStringLiteral("LogiKnotting.exe")};
    const AppTarget braidingTarget{QStringLiteral("LogiBraiding"),
                                   QStringLiteral("LogiBraiding"),
                                   QStringLiteral("LogiBraiding.exe")};

    QString knottingError;
    QString braidingError;

    const bool knottingOk = launchTarget(knottingTarget, &knottingError);
    const bool braidingOk = launchTarget(braidingTarget, &braidingError);

    if (knottingOk && braidingOk) {
        setStatus(QStringLiteral("LogiKnotting et LogiBraiding sont lances."));
        return;
    }

    QStringList errors;
    if (!knottingOk && !knottingError.isEmpty()) {
        errors << knottingError;
    }
    if (!braidingOk && !braidingError.isEmpty()) {
        errors << braidingError;
    }

    setStatus(errors.join(QStringLiteral(" | ")), true);
}

void MainWindow::buildBoth()
{
    const AppTarget knottingTarget{QStringLiteral("LogiKnotting"),
                                   QStringLiteral("LogiKnotting"),
                                   QStringLiteral("LogiKnotting.exe")};
    const AppTarget braidingTarget{QStringLiteral("LogiBraiding"),
                                   QStringLiteral("LogiBraiding"),
                                   QStringLiteral("LogiBraiding.exe")};

    QString error;
    if (!buildTarget(knottingTarget, &error)) {
        setStatus(error, true);
        return;
    }

    if (!buildTarget(braidingTarget, &error)) {
        setStatus(error, true);
        return;
    }

    setStatus(QStringLiteral("Compilation terminee pour LogiKnotting et LogiBraiding."));
}
