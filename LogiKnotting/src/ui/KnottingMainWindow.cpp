// ============================================================
// LogiKnotting
// Knot Design & Topology Software
// -------------------------------------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later // SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Norbert TRUPIANO
// This file is part of the LogiKnotting project.
// -------------------------------------------------------------------------------------------------
// Concept : Norbert TRUPIANO
// Author  : Norbert TRUPIANO
// Help Analyste & Programming : ChatGPT & Codex
// -------------------------------------------------------------------------------------------------
// LogiKnotting is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option)
// any later version.
// -------------------------------------------------------------------------------------------------
// LogiKnotting is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// -------------------------------------------------------------------------------------------------
// See the GNU General Public License for more details.
// https://www.gnu.org/licenses/gpl-3.0.html
// ============================================================
// Repository  : https://github.com/norberttrupiano-wq/LogiKnotting
// File        : src/ui/KnottingMainWindow.cpp
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#include "KnottingMainWindow.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QColor>
#include <QColorDialog>
#include <QCoreApplication>
#include <QDir>
#include <QDialog>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QGraphicsScene>
#include <QPageLayout>
#include <QPageSetupDialog>
#include <QPageSize>
#include <QPainter>
#include <QPen>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>
#include <QTranslator>
#include <QCryptographicHash>
#include <QDate>
#include <QDesktopServices>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QUrl>
#include <QUrlQuery>

#include <algorithm>
#include <cmath>

#include "../model/workspacemodel.h"
#include "WorkspaceView.h"
#include "WorkspaceScene.h"

namespace
{
struct LanguageDef
{
    const char* code;
    const char* endonym;
    const char* flag;
};

static const LanguageDef kLanguages[] = {
    {"fr", "Francais", u8"\U0001F1EB\U0001F1F7"},
    {"en", "English", u8"\U0001F1EC\U0001F1E7"},
    {"de", "Deutsch", u8"\U0001F1E9\U0001F1EA"},
    {"nl", "Nederlands", u8"\U0001F1F3\U0001F1F1"},
    {"nl_BE", "Vlaams", u8"\U0001F1E7\U0001F1EA"},
    {"es", "Espanol", u8"\U0001F1EA\U0001F1F8"},
    {"it", "Italiano", u8"\U0001F1EE\U0001F1F9"},
    {"pt", "Portugues", u8"\U0001F1F5\U0001F1F9"},
    {"pt_BR", "Portugues (Brasil)", u8"\U0001F1E7\U0001F1F7"},
    {"ja", u8"\u65e5\u672c\u8a9e", u8"\U0001F1EF\U0001F1F5"},
    {"zh_CN", u8"\u4e2d\u6587\uff08\u7b80\u4f53\uff09", u8"\U0001F1E8\U0001F1F3"},
    {"zh_TW", u8"\u4e2d\u6587\uff08\u7e41\u9ad4\uff09", u8"\U0001F1F9\U0001F1FC"},
    {"el", u8"\u0395\u03bb\u03bb\u03b7\u03bd\u03b9\u03ba\u03ac", u8"\U0001F1EC\U0001F1F7"},
    {"ru", u8"\u0420\u0443\u0441\u0441\u043a\u0438\u0439", u8"\U0001F1F7\U0001F1FA"},
    {"uk", u8"\u0423\u043a\u0440\u0430\u0457\u043d\u0441\u044c\u043a\u0430", u8"\U0001F1FA\U0001F1E6"},
    {"fi", "Suomi", u8"\U0001F1EB\U0001F1EE"},
    {"da", "Dansk", u8"\U0001F1E9\U0001F1F0"},
    {"sv", "Svenska", u8"\U0001F1F8\U0001F1EA"},
    {"no", "Norsk", u8"\U0001F1F3\U0001F1F4"},
    {"pl", "Polski", u8"\U0001F1F5\U0001F1F1"},
    {"cs", "Cestina", u8"\U0001F1E8\U0001F1FF"},
    {"sk", "Slovencina", u8"\U0001F1F8\U0001F1F0"},
    {"hu", "Magyar", u8"\U0001F1ED\U0001F1FA"},
    {"ro", "Romana", u8"\U0001F1F7\U0001F1F4"},
    {"bg", u8"\u0411\u044a\u043b\u0433\u0430\u0440\u0441\u043a\u0438", u8"\U0001F1E7\U0001F1EC"},
    {"tr", "Turkce", u8"\U0001F1F9\U0001F1F7"},
    {"ar", u8"\u0627\u0644\u0639\u0631\u0628\u064a\u0629", u8"\U0001F1F8\U0001F1E6"},
    {"he", u8"\u05e2\u05d1\u05e8\u05d9\u05ea", u8"\U0001F1EE\U0001F1F1"},
    {"ko", u8"\ud55c\uad6d\uc5b4", u8"\U0001F1F0\U0001F1F7"},
    {"vi", u8"Ti\u1ebfng Vi\u1ec7t", u8"\U0001F1FB\U0001F1F3"}
};


struct NotoFontCache
{
    bool baseLoaded = false;
    bool arabicLoaded = false;
    bool hebrewLoaded = false;
    bool jpLoaded = false;
    bool scLoaded = false;
    bool tcLoaded = false;
    bool krLoaded = false;

    QStringList baseFamilies;
    QStringList arabicFamilies;
    QStringList hebrewFamilies;
    QStringList jpFamilies;
    QStringList scFamilies;
    QStringList tcFamilies;
    QStringList krFamilies;
};

static NotoFontCache& notoCache()
{
    static NotoFontCache cache;
    return cache;
}

static void appendUnique(QStringList& dst, const QStringList& src)
{
    for (const QString& s : src)
    {
        if (!dst.contains(s, Qt::CaseInsensitive))
            dst.push_back(s);
    }
}

static QStringList candidateFontPaths(const QString& fileName)
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList roots = {
        QStringLiteral(":/fonts"),
        QDir(appDir).filePath(QStringLiteral("resources/fonts")),
        QDir(appDir).filePath(QStringLiteral("../resources/fonts")),
        QDir(appDir).filePath(QStringLiteral("../../resources/fonts")),
        QDir::current().filePath(QStringLiteral("resources/fonts")),
        QDir::current().filePath(QStringLiteral("../resources/fonts")),
        QDir::current().filePath(QStringLiteral("../../resources/fonts"))
    };

    QStringList out;
    out.reserve(roots.size());
    for (const QString& root : roots)
    {
        if (root.startsWith(":/"))
            out.push_back(root + QStringLiteral("/") + fileName);
        else
            out.push_back(QDir(root).filePath(fileName));
    }
    return out;
}

static QStringList tryLoadFontFamilies(const QString& fileName)
{
    QStringList families;

    for (const QString& path : candidateFontPaths(fileName))
    {
        const bool isResourcePath = path.startsWith(":/");
        if (!isResourcePath && !QFileInfo::exists(path))
            continue;
        if (isResourcePath && !QFile::exists(path))
            continue;

        const int id = QFontDatabase::addApplicationFont(path);
        if (id < 0)
            continue;

        appendUnique(families, QFontDatabase::applicationFontFamilies(id));
        if (!families.isEmpty())
            break;
    }

    return families;
}

static void ensureNotoFontsForLanguage(const QString& languageCode)
{
    NotoFontCache& cache = notoCache();

    if (!cache.baseLoaded)
    {
        cache.baseFamilies = tryLoadFontFamilies(QStringLiteral("NotoSans-Regular.ttf"));
        cache.baseLoaded = true;
    }

    if (languageCode == QStringLiteral("ar") && !cache.arabicLoaded)
    {
        cache.arabicFamilies = tryLoadFontFamilies(QStringLiteral("NotoSansArabic-Regular.ttf"));
        cache.arabicLoaded = true;
    }
    else if (languageCode == QStringLiteral("he") && !cache.hebrewLoaded)
    {
        cache.hebrewFamilies = tryLoadFontFamilies(QStringLiteral("NotoSansHebrew-Regular.ttf"));
        cache.hebrewLoaded = true;
    }
    else if (languageCode == QStringLiteral("ja") && !cache.jpLoaded)
    {
        cache.jpFamilies = tryLoadFontFamilies(QStringLiteral("NotoSansJP-Regular.otf"));
        cache.jpLoaded = true;
    }
    else if (languageCode == QStringLiteral("zh_CN") && !cache.scLoaded)
    {
        cache.scFamilies = tryLoadFontFamilies(QStringLiteral("NotoSansSC-Regular.otf"));
        cache.scLoaded = true;
    }
    else if (languageCode == QStringLiteral("zh_TW") && !cache.tcLoaded)
    {
        cache.tcFamilies = tryLoadFontFamilies(QStringLiteral("NotoSansTC-Regular.otf"));
        cache.tcLoaded = true;
    }
    else if (languageCode == QStringLiteral("ko") && !cache.krLoaded)
    {
        cache.krFamilies = tryLoadFontFamilies(QStringLiteral("NotoSansKR-Regular.otf"));
        cache.krLoaded = true;
    }
}

static QString pickFirstInstalledFamily(const QStringList& preferred)
{
    const QStringList installed = QFontDatabase::families();

    for (const QString& candidate : preferred)
    {
        for (const QString& family : installed)
        {
            if (candidate.compare(family, Qt::CaseInsensitive) == 0)
                return family;
        }
    }

    return QString();
}

static QFont pickLanguageFont(const QString& languageCode, const QFont& base)
{
    ensureNotoFontsForLanguage(languageCode);
    const NotoFontCache& cache = notoCache();

    QFont f(base);
    QStringList preferred;

    if (languageCode == QStringLiteral("ar"))
    {
        preferred << cache.arabicFamilies << QStringLiteral("Noto Sans Arabic")
                  << cache.baseFamilies << QStringLiteral("Noto Sans")
                  << QStringLiteral("Segoe UI") << QStringLiteral("Tahoma") << QStringLiteral("Arial");
    }
    else if (languageCode == QStringLiteral("he"))
    {
        preferred << cache.hebrewFamilies << QStringLiteral("Noto Sans Hebrew")
                  << cache.baseFamilies << QStringLiteral("Noto Sans")
                  << QStringLiteral("Segoe UI") << QStringLiteral("Arial");
    }
    else if (languageCode == QStringLiteral("ru") || languageCode == QStringLiteral("uk") || languageCode == QStringLiteral("bg"))
    {
        preferred << cache.baseFamilies << QStringLiteral("Noto Sans")
                  << QStringLiteral("Segoe UI") << QStringLiteral("Arial");
    }
    else if (languageCode == QStringLiteral("el"))
    {
        preferred << cache.baseFamilies << QStringLiteral("Noto Sans")
                  << QStringLiteral("Segoe UI") << QStringLiteral("Arial");
    }
    else if (languageCode == QStringLiteral("ja"))
    {
        preferred << cache.jpFamilies << QStringLiteral("Noto Sans JP")
                  << cache.baseFamilies << QStringLiteral("Noto Sans")
                  << QStringLiteral("Yu Gothic UI") << QStringLiteral("Meiryo") << QStringLiteral("MS UI Gothic");
    }
    else if (languageCode == QStringLiteral("zh_CN"))
    {
        preferred << cache.scFamilies << QStringLiteral("Noto Sans SC")
                  << cache.baseFamilies << QStringLiteral("Noto Sans")
                  << QStringLiteral("Microsoft YaHei UI") << QStringLiteral("Microsoft YaHei") << QStringLiteral("SimSun");
    }
    else if (languageCode == QStringLiteral("zh_TW"))
    {
        preferred << cache.tcFamilies << QStringLiteral("Noto Sans TC")
                  << cache.baseFamilies << QStringLiteral("Noto Sans")
                  << QStringLiteral("Microsoft JhengHei UI") << QStringLiteral("Microsoft JhengHei") << QStringLiteral("PMingLiU");
    }
    else if (languageCode == QStringLiteral("ko"))
    {
        preferred << cache.krFamilies << QStringLiteral("Noto Sans KR")
                  << cache.baseFamilies << QStringLiteral("Noto Sans")
                  << QStringLiteral("Malgun Gothic") << QStringLiteral("Gulim");
    }
    else
    {
        preferred << cache.baseFamilies << QStringLiteral("Noto Sans")
                  << QStringLiteral("Segoe UI") << QStringLiteral("Arial");
    }

    const QString family = pickFirstInstalledFamily(preferred);
    if (!family.isEmpty())
        f.setFamily(family);

    return f;
}

static QString languageLabel(const LanguageDef& lang)
{
    QString bcp47 = QString::fromLatin1(lang.code);
    bcp47.replace(QLatin1Char('_'), QLatin1Char('-'));

    const QString endonym = QString::fromUtf8(lang.endonym);
    const QString flag = QString::fromUtf8(lang.flag);

    if (!flag.isEmpty())
        return QStringLiteral("%1 %2 (%3)").arg(flag, endonym, bcp47);

    return QStringLiteral("%1 (%2)").arg(endonym, bcp47);
}

static int languageIndexForCode(const QString& code)
{
    int index = 0;
    for (const auto& lang : kLanguages)
    {
        if (code == QString::fromLatin1(lang.code))
            return index;
        ++index;
    }
    return -1;
}

static QString endonymForCode(const QString& code)
{
    for (const auto& lang : kLanguages)
    {
        if (code == QString::fromLatin1(lang.code))
            return QString::fromUtf8(lang.endonym);
    }
    return code;
}

static constexpr int kTrialPeriodDays = 15;
static const char* kRegistrationInboxEmail = "CHANGE_ME@example.com";
static const char* kRegistrationSalt = "KeyGenerator::SidoineArifene::v1";

static QSettings appSettings()
{
    return QSettings(QSettings::NativeFormat,
                     QSettings::UserScope,
                     QStringLiteral("LogiKnotting"),
                     QStringLiteral("LogiKnotting"));
}

static QString lastWorkspaceFileSettingKey()
{
    return QStringLiteral("ui/last_workspace_file");
}

static QString releaseWorkspaceDirectoryCandidate()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("KnotBraid"));
}

static QString documentsWorkspaceDirectoryCandidate()
{
    QString documentsDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (documentsDir.trimmed().isEmpty())
        documentsDir = QDir::homePath();

    return QDir(documentsDir).filePath(QStringLiteral("KnotBraid"));
}

static bool ensureDirectoryExists(const QString& directoryPath)
{
    if (directoryPath.trimmed().isEmpty())
        return false;

    QDir dir(directoryPath);
    if (dir.exists())
        return true;

    return QDir().mkpath(directoryPath);
}

static bool isWritableDirectoryReady(const QString& directoryPath)
{
    if (!ensureDirectoryExists(directoryPath))
        return false;

    const QFileInfo info(directoryPath);
    return info.exists() && info.isDir() && info.isWritable();
}

static QString canonicalEmail(const QString& email)
{
    return email.trimmed().toLower();
}

static QString authorIdFromEmail(const QString& email)
{
    const QString normalized = canonicalEmail(email);
    if (normalized.isEmpty())
        return QString();

    const QByteArray digest = QCryptographicHash::hash(normalized.toUtf8(), QCryptographicHash::Sha256)
                                  .toHex()
                                  .toUpper();
    return QString::fromLatin1(digest.left(12));
}

static bool isValidEmail(const QString& email)
{
    static const QRegularExpression re(
        QStringLiteral(R"(^[A-Z0-9._%+\-]+@[A-Z0-9.\-]+\.[A-Z]{2,}$)"),
        QRegularExpression::CaseInsensitiveOption);

    return re.match(canonicalEmail(email)).hasMatch();
}

static QString normalizeUnlockKey(const QString& key)
{
    QString out;
    out.reserve(key.size());
    for (const QChar ch : key.toUpper())
    {
        if (ch.isDigit() || (ch >= QLatin1Char('A') && ch <= QLatin1Char('Z')))
            out.push_back(ch);
    }
    return out;
}

// Key algorithm mirrored from KeyGenerator::LicenseCore::generateKey().
static QString generateUnlockKeyForEmail(const QString& email)
{
    const QString normalizedEmail = canonicalEmail(email);
    const QByteArray data = (normalizedEmail + QString::fromLatin1(kRegistrationSalt)).toUtf8();
    const QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex();

    QString key;
    for (int i = 0; i < 16; ++i)
    {
        key.append(QChar::fromLatin1(hash[i]));
        if (((i + 1) % 4) == 0 && i != 15)
            key.append(QLatin1Char('-'));
    }

    return key.toUpper();
}

static bool verifyUnlockKey(const QString& email, const QString& key)
{
    if (!isValidEmail(email))
        return false;

    const QString expected = normalizeUnlockKey(generateUnlockKeyForEmail(email));
    const QString provided = normalizeUnlockKey(key);
    return !provided.isEmpty() && (expected == provided);
}

static QDate ensureTrialStartDate(QSettings& s)
{
    const QString key = QStringLiteral("registration/trial_start_utc");
    QDate d = QDate::fromString(s.value(key).toString(), Qt::ISODate);
    const QDate todayUtc = QDate::currentDate();

    if (!d.isValid() || d > todayUtc)
    {
        d = todayUtc;
        s.setValue(key, d.toString(Qt::ISODate));
    }

    return d;
}

static int trialDaysRemaining(QSettings& s)
{
    const QDate start = ensureTrialStartDate(s);
    const int elapsedDays = start.daysTo(QDate::currentDate());
    return kTrialPeriodDays - elapsedDays;
}

static bool isRegistrationUnlocked(QSettings& s)
{
    const QString email = canonicalEmail(s.value(QStringLiteral("registration/email")).toString());
    const bool unlocked = s.value(QStringLiteral("registration/unlocked"), false).toBool();
    if (unlocked && isValidEmail(email))
        return true;

    // Backward compatibility: migrate legacy stored unlock key to boolean flag.
    const QString legacyKey = s.value(QStringLiteral("registration/unlock_key")).toString().trimmed();
    if (isValidEmail(email) && !legacyKey.isEmpty() && verifyUnlockKey(email, legacyKey))
    {
        s.setValue(QStringLiteral("registration/unlocked"), true);
        s.remove(QStringLiteral("registration/unlock_key"));
        s.sync();
        return true;
    }

    return false;
}
} // namespace

namespace LogiKnottingApp {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    if (parent != nullptr)
        setWindowFlags(Qt::Widget);

    m_translator = new QTranslator(this);
    m_defaultAppFont = qApp->font();

    loadPrintProfileSettings();

    m_printer = new QPrinter(QPrinter::HighResolution);
    m_printer->setFullPage(false);
    applyPrintProfileToPrinter();

    buildUi();
    buildMenusAndShortcuts();
    buildStatusBar();

    bool restoredLastWorkspace = false;
    const QString lastWorkspacePath = lastOpenedWorkspacePath();
    if (!lastWorkspacePath.isEmpty())
    {
        restoredLastWorkspace = loadWorkspaceFromPath(lastWorkspacePath, false, true);
        if (!restoredLastWorkspace)
            clearLastOpenedWorkspacePath();
    }

    if (!restoredLastWorkspace && m_model)
    {
        m_model->initializeAuditForNewDocument(currentAuthorId());
        m_lastAuditCheckpointSeconds = m_model->designTimeSeconds();
    }

    if (!restoredLastWorkspace)
        statusBar()->showMessage(tr("Pr\u00EAt"));

    if (!ensureTrialAccess())
        QTimer::singleShot(0, qApp, &QCoreApplication::quit);
}

MainWindow::~MainWindow()
{
    delete m_view;
    delete m_model;
    delete m_printer;
}
WorkspaceView* MainWindow::workspaceView() const
{
    return m_view;
}

void MainWindow::loadPrintProfileSettings()
{
    QSettings s = appSettings();

    const QString paper = s.value(QStringLiteral("print/profile/paper"), QStringLiteral("A4"))
                              .toString()
                              .trimmed()
                              .toUpper();
    m_printPaperA3 = (paper == QStringLiteral("A3"));

    int overlap = s.value(QStringLiteral("print/profile/overlap_mm"), 8).toInt();
    if (overlap != 5 && overlap != 8 && overlap != 10)
        overlap = 8;
    m_printOverlapMM = overlap;

    m_printHideGrid = s.value(QStringLiteral("print/profile/hide_grid"), false).toBool();
    m_printCenterHorizontally = s.value(QStringLiteral("print/profile/center_horizontal"), false).toBool();
    m_printCenterVertically = s.value(QStringLiteral("print/profile/center_vertical"), false).toBool();
}

void MainWindow::savePrintProfileSettings() const
{
    QSettings s = appSettings();
    s.setValue(QStringLiteral("print/profile/paper"), m_printPaperA3 ? QStringLiteral("A3") : QStringLiteral("A4"));
    s.setValue(QStringLiteral("print/profile/overlap_mm"), m_printOverlapMM);
    s.setValue(QStringLiteral("print/profile/hide_grid"), m_printHideGrid);
    s.setValue(QStringLiteral("print/profile/center_horizontal"), m_printCenterHorizontally);
    s.setValue(QStringLiteral("print/profile/center_vertical"), m_printCenterVertically);
    s.sync();
}

void MainWindow::applyPrintProfileToPrinter()
{
    if (!m_printer)
        return;

    QPageLayout layout = m_printer->pageLayout();
    layout.setPageSize(QPageSize(m_printPaperA3 ? QPageSize::A3 : QPageSize::A4));
    layout.setOrientation(QPageLayout::Landscape);
    m_printer->setPageLayout(layout);
}

QString MainWindow::printPaperProfileLabel() const
{
    return m_printPaperA3 ? tr("A3") : tr("A4");
}

QString MainWindow::lastOpenedWorkspacePath() const
{
    QSettings s = appSettings();
    return s.value(lastWorkspaceFileSettingKey()).toString().trimmed();
}

QString MainWindow::defaultWorkspaceDirectory() const
{
    const QString releaseDir = QDir::cleanPath(releaseWorkspaceDirectoryCandidate());
    if (isWritableDirectoryReady(releaseDir))
        return releaseDir;

    const QString documentsDir = QDir::cleanPath(documentsWorkspaceDirectoryCandidate());
    if (ensureDirectoryExists(documentsDir))
        return documentsDir;

    return QDir::cleanPath(QCoreApplication::applicationDirPath());
}

void MainWindow::saveLastOpenedWorkspacePath() const
{
    if (m_currentFilePath.trimmed().isEmpty())
        return;

    QSettings s = appSettings();
    s.setValue(lastWorkspaceFileSettingKey(), m_currentFilePath);
}

void MainWindow::clearLastOpenedWorkspacePath() const
{
    QSettings s = appSettings();
    s.remove(lastWorkspaceFileSettingKey());
}

void MainWindow::showDefaultWorkspaceDirectoryDialog()
{
    const QString activeDirectory = defaultWorkspaceDirectory();
    const QString releaseDirectory = QDir::cleanPath(releaseWorkspaceDirectoryCandidate());
    const QString documentsDirectory = QDir::cleanPath(documentsWorkspaceDirectoryCandidate());
    const bool usesReleaseDirectory = (activeDirectory.compare(releaseDirectory, Qt::CaseInsensitive) == 0);

    QDialog dialog(this);
    dialog.setWindowTitle(tr("R\u00E9pertoire par d\u00E9faut"));
    dialog.setModal(true);
    dialog.resize(720, 220);

    auto* layout = new QVBoxLayout(&dialog);

    auto* infoLabel = new QLabel(
        tr("Les fichiers LogiKnotting s'ouvrent et se proposent par d\u00E9faut dans ce r\u00E9pertoire. "
           "Le dossier pr\u00E8s du Release est utilis\u00E9 s'il est disponible et accessible en \u00E9criture ; "
           "sinon le dossier Documents/KnotBraid prend le relais."),
        &dialog);
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);

    auto* formLayout = new QFormLayout();

    auto* activeEdit = new QLineEdit(activeDirectory, &dialog);
    activeEdit->setReadOnly(true);
    formLayout->addRow(tr("R\u00E9pertoire actif"), activeEdit);

    auto* modeEdit = new QLineEdit(usesReleaseDirectory ? tr("Release/KnotBraid") : tr("Documents/KnotBraid"), &dialog);
    modeEdit->setReadOnly(true);
    formLayout->addRow(tr("Source active"), modeEdit);

    auto* releaseEdit = new QLineEdit(releaseDirectory, &dialog);
    releaseEdit->setReadOnly(true);
    formLayout->addRow(tr("Dossier Release"), releaseEdit);

    auto* documentsEdit = new QLineEdit(documentsDirectory, &dialog);
    documentsEdit->setReadOnly(true);
    formLayout->addRow(tr("Dossier Documents"), documentsEdit);

    layout->addLayout(formLayout);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok, &dialog);
    auto* openButton = buttons->addButton(tr("Ouvrir le dossier"), QDialogButtonBox::ActionRole);
    connect(openButton, &QPushButton::clicked, &dialog, [activeDirectory]() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(activeDirectory));
    });
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    layout->addWidget(buttons);

    dialog.exec();
}

bool MainWindow::loadWorkspaceFromPath(const QString& path, bool showWarning, bool announceStatus)
{
    if (!m_model)
        return false;

    const QString normalizedPath = path.trimmed();
    if (normalizedPath.isEmpty() || !QFileInfo::exists(normalizedPath))
        return false;

    if (!m_model->loadFromFile(normalizedPath))
    {
        if (showWarning)
        {
            QMessageBox::warning(
                this,
                tr("Erreur"),
                tr("Echec du chargement.\nFichier non conforme ou modifie manuellement."));
        }
        return false;
    }

    const bool validatedFile = normalizedPath.endsWith(QStringLiteral(".lkv"), Qt::CaseInsensitive);

    m_currentFilePath = normalizedPath;
    m_lastAuditCheckpointSeconds = m_model->designTimeSeconds();
    setReadOnlyUiState(validatedFile);
    if (m_view)
        m_view->clearSketchOverlay();
    updateWindowTitle();

    if (m_view && m_view->scene())
    {
        m_view->scene()->update();
        m_view->viewport()->update();
    }

    refreshStatusBar();
    refreshActiveRopeUi();
    saveLastOpenedWorkspacePath();

    if (announceStatus && statusBar())
        statusBar()->showMessage(tr("Ouvert : %1").arg(normalizedPath), 3000);

    return true;
}

void MainWindow::buildUi()
{
    m_model = new Model::WorkspaceModel();

    m_view = new WorkspaceView(this);
    m_view->setModel(m_model);

    connect(m_view,
            &WorkspaceView::mousePositionChanged,
            this,
            &MainWindow::onSnapMoved);

    connect(m_view,
            &WorkspaceView::crossingEditModeChanged,
            this,
            [this](bool enabled)
            {
                if (m_labelMode)
                {
                    m_labelMode->setText(enabled ? tr("Mode : Insert (croisements)")
                                                 : tr("Mode : Tra\u00E7age"));
                    m_labelMode->setStyleSheet(enabled
                                                   ? QStringLiteral("QLabel { color: #b36b00; font-weight: 600; }")
                                                   : QString());
                }

                if (statusBar())
                {
                    statusBar()->showMessage(enabled
                                                 ? tr("Mode Insert actif : clic droit pour inverser un croisement")
                                                 : tr("Mode Insert d\u00E9sactiv\u00E9"),
                                             2000);
                }
            });

    setCentralWidget(m_view);
    updateWindowTitle();
    if (parentWidget() == nullptr)
        showMaximized();
}

void MainWindow::updateWindowTitle()
{
    const QString name =
        m_currentFilePath.isEmpty() ? tr("Noname.lkw") : QFileInfo(m_currentFilePath).fileName();

    setWindowTitle(tr("LogiKnotting - %1").arg(name));
}

void MainWindow::buildMenusAndShortcuts()
{
    menuBar()->clear();

    if (m_ropeActionGroup)
    {
        m_ropeActionGroup->deleteLater();
        m_ropeActionGroup = nullptr;
    }
    m_ropeActions.clear();

    m_actionSave = nullptr;
    m_actionValidation = nullptr;
    m_actionCopy = nullptr;
    m_actionPageSetup = nullptr;
    m_actionPrintPreview = nullptr;
    m_actionPrint = nullptr;
    m_actionSketchMode = nullptr;
    m_actionBreakSketch = nullptr;
    m_actionTracingMode = nullptr;
    m_actionRotateRight45 = nullptr;
    m_actionInvertDirection = nullptr;
    m_actionZoomIn = nullptr;
    m_actionZoomOut = nullptr;
    m_actionZoomReset = nullptr;
    m_actionPlayAnimation = nullptr;

    QMenu* fileMenu = menuBar()->addMenu(tr("&Fichier"));

    QAction* newAction = fileMenu->addAction(tr("Nouveau"));
    connect(newAction, &QAction::triggered, this, &MainWindow::onNewFile);

    m_actionSave = fileMenu->addAction(tr("&Enregistrer"));
    m_actionSave->setShortcut(QKeySequence::Save);
    connect(m_actionSave, &QAction::triggered, this, &MainWindow::onSave);

    QAction* actOpen = fileMenu->addAction(tr("&Ouvrir"));
    actOpen->setShortcut(QKeySequence::Open);
    connect(actOpen, &QAction::triggered, this, &MainWindow::onOpen);

    m_actionPageSetup = fileMenu->addAction(tr("Mise en page..."));
    connect(m_actionPageSetup, &QAction::triggered, this, &MainWindow::onPageSetup);

    m_actionPrintPreview = fileMenu->addAction(tr("Apercu avant impression"));
    connect(m_actionPrintPreview, &QAction::triggered, this, &MainWindow::onPrintPreview);

    m_actionPrint = fileMenu->addAction(tr("Imprimer..."));
    m_actionPrint->setShortcut(QKeySequence::Print);
    connect(m_actionPrint, &QAction::triggered, this, &MainWindow::onPrint);

    QMenu* printProfileMenu = fileMenu->addMenu(tr("Profil d'impression"));

    QMenu* paperMenu = printProfileMenu->addMenu(tr("Format papier"));
    QActionGroup* paperGroup = new QActionGroup(paperMenu);
    paperGroup->setExclusive(true);

    QAction* actPaperA4 = paperMenu->addAction(tr("A4"));
    actPaperA4->setCheckable(true);
    actPaperA4->setChecked(!m_printPaperA3);
    paperGroup->addAction(actPaperA4);
    connect(actPaperA4, &QAction::triggered, this, [this]() {
        if (m_printPaperA3)
        {
            m_printPaperA3 = false;
            applyPrintProfileToPrinter();
            savePrintProfileSettings();
            if (statusBar())
                statusBar()->showMessage(tr("Profil impression : %1, recouvrement %2 mm")
                                             .arg(printPaperProfileLabel())
                                             .arg(m_printOverlapMM),
                                         2500);
        }
    });

    QAction* actPaperA3 = paperMenu->addAction(tr("A3"));
    actPaperA3->setCheckable(true);
    actPaperA3->setChecked(m_printPaperA3);
    paperGroup->addAction(actPaperA3);
    connect(actPaperA3, &QAction::triggered, this, [this]() {
        if (!m_printPaperA3)
        {
            m_printPaperA3 = true;
            applyPrintProfileToPrinter();
            savePrintProfileSettings();
            if (statusBar())
                statusBar()->showMessage(tr("Profil impression : %1, recouvrement %2 mm")
                                             .arg(printPaperProfileLabel())
                                             .arg(m_printOverlapMM),
                                         2500);
        }
    });

    QMenu* overlapMenu = printProfileMenu->addMenu(tr("Recouvrement"));
    QActionGroup* overlapGroup = new QActionGroup(overlapMenu);
    overlapGroup->setExclusive(true);

    for (const int mm : {5, 8, 10})
    {
        QAction* act = overlapMenu->addAction(tr("%1 mm").arg(mm));
        act->setCheckable(true);
        act->setChecked(m_printOverlapMM == mm);
        overlapGroup->addAction(act);

        connect(act, &QAction::triggered, this, [this, mm]() {
            if (m_printOverlapMM == mm)
                return;

            m_printOverlapMM = mm;
            savePrintProfileSettings();
            if (statusBar())
                statusBar()->showMessage(tr("Profil impression : %1, recouvrement %2 mm")
                                             .arg(printPaperProfileLabel())
                                             .arg(m_printOverlapMM),
                                         2500);
        });
    }

    QMenu* centeringMenu = printProfileMenu->addMenu(tr("Centrage"));
    auto showCenteringStatus = [this]()
    {
        if (!statusBar())
            return;

        QString mode = tr("desactive");
        if (m_printCenterHorizontally && m_printCenterVertically)
            mode = tr("horizontal + vertical");
        else if (m_printCenterHorizontally)
            mode = tr("horizontal");
        else if (m_printCenterVertically)
            mode = tr("vertical");

        statusBar()->showMessage(tr("Profil impression : centrage %1").arg(mode), 2500);
    };

    QAction* actCenterHorizontal = centeringMenu->addAction(tr("Horizontal"));
    actCenterHorizontal->setCheckable(true);
    actCenterHorizontal->setChecked(m_printCenterHorizontally);
    connect(actCenterHorizontal, &QAction::toggled, this, [this, showCenteringStatus](bool checked) {
        m_printCenterHorizontally = checked;
        savePrintProfileSettings();
        showCenteringStatus();
    });

    QAction* actCenterVertical = centeringMenu->addAction(tr("Vertical"));
    actCenterVertical->setCheckable(true);
    actCenterVertical->setChecked(m_printCenterVertically);
    connect(actCenterVertical, &QAction::toggled, this, [this, showCenteringStatus](bool checked) {
        m_printCenterVertically = checked;
        savePrintProfileSettings();
        showCenteringStatus();
    });

    printProfileMenu->addSeparator();

    QAction* actHideGrid = printProfileMenu->addAction(tr("Sans grille (points et segments)"));
    actHideGrid->setCheckable(true);
    actHideGrid->setChecked(m_printHideGrid);
    connect(actHideGrid, &QAction::toggled, this, [this](bool checked) {
        m_printHideGrid = checked;
        savePrintProfileSettings();
        if (statusBar())
            statusBar()->showMessage(tr("Profil impression : %1")
                                         .arg(m_printHideGrid ? tr("sans grille") : tr("avec grille")),
                                     2500);
    });

    fileMenu->addSeparator();

    m_actionValidation = fileMenu->addAction(tr("Validation"));
    connect(m_actionValidation, &QAction::triggered, this, &MainWindow::onValidate);

    m_actionCopy = fileMenu->addAction(tr("Copie"));
    connect(m_actionCopy, &QAction::triggered, this, &MainWindow::onCopyFromValidated);

    fileMenu->addSeparator();

    QAction* filePropsAction = fileMenu->addAction(tr("Proprietes du fichier"));
    connect(filePropsAction, &QAction::triggered, this, &MainWindow::showFilePropertiesDialog);

    const QString viewMenuTitle = (m_currentLanguageCode == QStringLiteral("fr")) ? tr("Vue") : tr("View");
    QMenu* viewMenu = menuBar()->addMenu(viewMenuTitle);

    m_actionZoomIn = viewMenu->addAction(tr("Zoom +"));
    m_actionZoomIn->setShortcut(QKeySequence(QStringLiteral("Ctrl++")));
    connect(m_actionZoomIn, &QAction::triggered, this, [this]() {
        if (m_view)
            m_view->zoomInView();
    });

    m_actionZoomOut = viewMenu->addAction(tr("Zoom -"));
    m_actionZoomOut->setShortcut(QKeySequence(QStringLiteral("Ctrl+-")));
    connect(m_actionZoomOut, &QAction::triggered, this, [this]() {
        if (m_view)
            m_view->zoomOutView();
    });

    m_actionZoomReset = viewMenu->addAction(tr("Zoom 100%"));
    m_actionZoomReset->setShortcut(QKeySequence(QStringLiteral("Ctrl+0")));
    connect(m_actionZoomReset, &QAction::triggered, this, [this]() {
        if (m_view)
            m_view->zoomResetView();
    });

    viewMenu->addSeparator();

    m_actionPlayAnimation = viewMenu->addAction(tr("Play Animation"));
    connect(m_actionPlayAnimation, &QAction::triggered, this, [this]() {
        if (m_view)
            m_view->startAnimation();
    });
    QMenu* sketchMenu = menuBar()->addMenu(tr("Esquisse"));

    m_actionRotateRight45 = sketchMenu->addAction(style()->standardIcon(QStyle::SP_BrowserReload), tr("Rotation de 45 a droite"));
    connect(m_actionRotateRight45, &QAction::triggered, m_view, &WorkspaceView::rotateSelectionRight45);

    m_actionInvertDirection = sketchMenu->addAction(style()->standardIcon(QStyle::SP_ArrowBack), tr("Inversion de sens"));
    connect(m_actionInvertDirection, &QAction::triggered, m_view, &WorkspaceView::invertSelectionDirection);

    sketchMenu->addSeparator();

    m_actionSketchMode = sketchMenu->addAction(tr("Mode Esquisse"));
    m_actionSketchMode->setShortcut(QKeySequence("Ctrl+E"));
    connect(m_actionSketchMode, &QAction::triggered, m_view, &WorkspaceView::enterSketchMode);

    m_actionBreakSketch = sketchMenu->addAction(tr("Sectionner l'esquisse"));
    m_actionBreakSketch->setShortcut(QKeySequence(Qt::Key_Space));
    connect(m_actionBreakSketch, &QAction::triggered, m_view, &WorkspaceView::breakSketch);

    sketchMenu->addSeparator();

    m_actionTracingMode = sketchMenu->addAction(tr("Mode Tracage"));
    m_actionTracingMode->setShortcut(QKeySequence("Ctrl+B"));
    connect(m_actionTracingMode, &QAction::triggered, m_view, &WorkspaceView::enterTracingMode);

    QMenu* ropesMenu = menuBar()->addMenu(tr("Cordes"));
    m_ropeActionGroup = new QActionGroup(this);
    m_ropeActionGroup->setExclusive(true);

    const auto& topo = m_model->topologySnapshot();

    for (int i = 0; i < static_cast<int>(Domain::MaxRopes); ++i)
    {
        QColor c(40, 120, 255);
        if (static_cast<std::size_t>(i) < topo.ropes.size())
            c = topo.ropes[static_cast<std::size_t>(i)].color;

        QPixmap pm(14, 14);
        pm.fill(c);

        QAction* actRope = ropesMenu->addAction(QIcon(pm), tr("Corde %1").arg(i + 1));
        actRope->setCheckable(true);
        actRope->setShortcut(QKeySequence(QStringLiteral("Ctrl+%1").arg(i + 1)));
        if (i == m_model->activeRopeId())
            actRope->setChecked(true);

        connect(actRope, &QAction::triggered, this, [this, i]() {
            if (!m_model)
                return;

            m_model->setActiveRopeId(i);
            refreshActiveRopeUi();
        });

        m_ropeActionGroup->addAction(actRope);
        m_ropeActions.push_back(actRope);
    }

    buildLanguageMenu();
    setReadOnlyUiState(m_currentFileValidated);
    updateFileActionsState();
}
void MainWindow::buildLanguageMenu()
{
    QMenu* settingsMenu = menuBar()->addMenu(tr("Param\u00E8tres"));

    QAction* languageAction = settingsMenu->addAction(tr("Langues"));
    connect(languageAction, &QAction::triggered, this, &MainWindow::openLanguageDialog);

    QMenu* ropeColorsMenu = settingsMenu->addMenu(tr("Couleurs des cordes"));
    for (int i = 0; i < static_cast<int>(Domain::MaxRopes); ++i)
    {
        const QColor color = m_model ? m_model->ropeColor(i) : QColor(40, 120, 255);
        QPixmap pm(14, 14);
        pm.fill(color);

        QAction* colorAction = ropeColorsMenu->addAction(QIcon(pm), tr("Corde %1...").arg(i + 1));
        connect(colorAction, &QAction::triggered, this, [this, i]() {
            if (!m_model)
                return;

            const QColor chosen = QColorDialog::getColor(
                m_model->ropeColor(i),
                this,
                tr("Couleur corde %1").arg(i + 1)
            );

            if (!chosen.isValid())
                return;

            m_model->setRopeColor(i, chosen);

            if (m_view && m_view->scene())
            {
                m_view->scene()->update();
                if (m_view->viewport())
                    m_view->viewport()->update();
            }

            refreshActiveRopeUi();
            buildMenusAndShortcuts();
        });
    }

    QAction* filesDirectoryAction = settingsMenu->addAction(tr("R\u00E9pertoire des fichiers"));
    connect(filesDirectoryAction, &QAction::triggered, this, &MainWindow::showDefaultWorkspaceDirectoryDialog);

    settingsMenu->addSeparator();

    QAction* helpAction = settingsMenu->addAction(tr("Aide"));
    connect(helpAction, &QAction::triggered, this, &MainWindow::showHelpDialog);

    QAction* aboutAction = settingsMenu->addAction(tr("\u00C0 Propos"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);
}

void MainWindow::openLanguageDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Langues"));

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QComboBox* combo = new QComboBox(&dialog);
    combo->setMinimumWidth(420);

    for (const auto& lang : kLanguages)
    {
        const QString code = QString::fromLatin1(lang.code);
        combo->addItem(languageLabel(lang), code);
    }

    const int currentIndex = languageIndexForCode(m_currentLanguageCode);
    if (currentIndex >= 0)
        combo->setCurrentIndex(currentIndex);

    layout->addWidget(combo);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted)
        return;

    const QString selectedCode = combo->currentData().toString();
    if (!selectedCode.isEmpty() && selectedCode != m_currentLanguageCode)
        applyLanguage(selectedCode);
}

void MainWindow::showHelpDialog()
{
    QMessageBox::information(
        this,
        tr("Aide"),
        tr("Consultez le code source du projet :\nhttps://github.com/<username>/LogiKnotting")
    );
}

bool MainWindow::openRegistrationDialog(bool trialExpired)
{
    QSettings s = appSettings();

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Inscription"));
    dialog.setModal(true);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    QLabel* intro = new QLabel(
        trialExpired
            ? tr("La periode d'essai de 15 jours est expiree. Entrez une cle de debridage pour continuer.")
            : tr("Entrez un email valide pour demander une inscription, ou collez directement votre cle de debridage."),
        &dialog);
    intro->setWordWrap(true);
    layout->addWidget(intro);

    QFormLayout* form = new QFormLayout();
    QLineEdit* emailEdit = new QLineEdit(&dialog);
    emailEdit->setPlaceholderText(QStringLiteral("name@example.com"));
    emailEdit->setText(canonicalEmail(s.value(QStringLiteral("registration/pending_email")).toString()));
    form->addRow(tr("Email"), emailEdit);

    QLineEdit* keyEdit = new QLineEdit(&dialog);
    keyEdit->setPlaceholderText(QStringLiteral("XXXX-XXXX-XXXX-XXXX"));
    form->addRow(tr("Cle de debridage"), keyEdit);
    layout->addLayout(form);

    QLabel* help = new QLabel(
        tr("Validation avec email: ouverture de votre client mail vers la boite d'inscription.\nValidation avec cle: debloque immediatement l'application."),
        &dialog);
    help->setWordWrap(true);
    layout->addWidget(help);

    QDialogButtonBox* buttons = new QDialogButtonBox(&dialog);
    QPushButton* validateBtn = buttons->addButton(tr("Valider"), QDialogButtonBox::AcceptRole);
    buttons->addButton(trialExpired ? tr("Quitter") : tr("Fermer"), QDialogButtonBox::RejectRole);
    layout->addWidget(buttons);

    QObject::connect(validateBtn, &QPushButton::clicked, &dialog, [this, &dialog, emailEdit, keyEdit]() {
        QSettings sLocal = appSettings();

        const QString emailInput = canonicalEmail(emailEdit->text());
        const QString keyInput = keyEdit->text().trimmed();

        if (!keyInput.isEmpty())
        {
            QString emailForKey = emailInput;
            if (emailForKey.isEmpty())
                emailForKey = canonicalEmail(sLocal.value(QStringLiteral("registration/pending_email")).toString());

            if (!isValidEmail(emailForKey))
            {
                QMessageBox::warning(this, tr("Inscription"), tr("Entrez d'abord un email valide associe a cette cle."));
                return;
            }

            if (!verifyUnlockKey(emailForKey, keyInput))
            {
                QMessageBox::warning(this, tr("Inscription"), tr("Cle invalide pour cet email."));
                return;
            }

            sLocal.setValue(QStringLiteral("registration/email"), emailForKey);
            sLocal.setValue(QStringLiteral("registration/unlocked"), true);
            sLocal.remove(QStringLiteral("registration/pending_email"));
            sLocal.sync();

            QMessageBox::information(this, tr("Inscription"), tr("Inscription validee. Application debridee."));
            dialog.accept();
            return;
        }

        if (!isValidEmail(emailInput))
        {
            QMessageBox::warning(this, tr("Inscription"), tr("Entrez un email valide ou une cle."));
            return;
        }

        sLocal.setValue(QStringLiteral("registration/pending_email"), emailInput);
        sLocal.sync();

        const QString inbox = QString::fromLatin1(kRegistrationInboxEmail).trimmed();
        if (inbox.isEmpty() || inbox.startsWith(QStringLiteral("CHANGE_ME"), Qt::CaseInsensitive))
        {
            QMessageBox::information(
                this,
                tr("Inscription"),
                tr("Email d'inscription non configure dans le code.\nRemplacez kRegistrationInboxEmail dans MainWindow.cpp puis recompilez."));
            return;
        }

        QUrl mailUrl;
        mailUrl.setScheme(QStringLiteral("mailto"));
        mailUrl.setPath(inbox);

        QUrlQuery query;
        query.addQueryItem(QStringLiteral("subject"), tr("Demande d'inscription LogiKnotting"));
        query.addQueryItem(QStringLiteral("body"),
                           tr("Bonjour,\n\nMerci de m'envoyer une cle de debridage pour cet email :\n%1\n\nCordialement,")
                               .arg(emailInput));
        mailUrl.setQuery(query);

        if (!QDesktopServices::openUrl(mailUrl))
        {
            QMessageBox::warning(this, tr("Inscription"), tr("Impossible d'ouvrir le client email par defaut."));
            return;
        }

        QMessageBox::information(
            this,
            tr("Inscription"),
            tr("Demande envoyee via votre client email.\nQuand vous recevez la cle, revenez ici et collez-la dans le champ cle."));
    });

    if (dialog.exec() != QDialog::Accepted)
        return false;

    QSettings check = appSettings();
    return isRegistrationUnlocked(check);
}

bool MainWindow::ensureTrialAccess()
{
    QSettings s = appSettings();
    if (isRegistrationUnlocked(s))
        return true;

    const int remaining = trialDaysRemaining(s);
    if (remaining > 0)
    {
        if (statusBar())
            statusBar()->showMessage(tr("Version d'essai: %1 jour(s) restant(s)").arg(remaining), 5000);
        return true;
    }

    QMessageBox::warning(this,
                         tr("Periode d'essai expiree"),
                         tr("La periode d'essai de 15 jours est terminee.\nUne inscription est necessaire pour continuer."));
    return openRegistrationDialog(true);
}

QString MainWindow::currentRegisteredEmail() const
{
    QSettings s = appSettings();
    const QString email = canonicalEmail(s.value(QStringLiteral("registration/email")).toString());
    return isValidEmail(email) ? email : QString();
}

QString MainWindow::currentAuthorId() const
{
    const QString email = currentRegisteredEmail();
    if (email.isEmpty())
        return QString();

    return authorIdFromEmail(email);
}

void MainWindow::showFilePropertiesDialog()
{
    if (!m_model)
        return;

    QMessageBox::information(this, tr("Proprietes du fichier"), m_model->filePropertiesText());
}

void MainWindow::showAboutDialog()
{
    const QString text = QStringLiteral(
        "LogiKnotting\n\n"
        "Version 1.0.0\n\n"
        "LogiKnotting is an open-source application for designing and analysing\n"
        "topological rope knots.\n\n"
        "Author\n"
        "Norbert TRUPIANO\n\n"
        "Concept\n"
        "Norbert TRUPIANO\n\n"
        "Analysis and Programming Assistance\n"
        "ChatGPT & Codex\n\n"
        "License\n"
        "GNU General Public License v3 (GPL-3.0)\n\n"
        "Source Code\n"
        "https://github.com/norberttrupiano-wq/LogiKnotting\n\n"
        "Third-party components\n"
        "Qt Framework\n"
        "Noto Fonts (Google)\n\n"
        "\u00A9 2026 Norbert TRUPIANO"
    );

    QSettings s = appSettings();
    const bool unlocked = isRegistrationUnlocked(s);
    const int remaining = trialDaysRemaining(s);
    const int shownRemaining = (remaining > 0) ? remaining : 0;

    const QString statusText = unlocked
        ? tr("\n\nInscription : active")
        : tr("\n\nEssai : %1 jour(s) restant(s)").arg(shownRemaining);

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setWindowTitle(tr("\u00C0 Propos"));
    box.setText(text + statusText);
    QPushButton* registerButton = box.addButton(tr("Inscription"), QMessageBox::ActionRole);
    box.addButton(QMessageBox::Close);
    box.exec();

    if (box.clickedButton() == registerButton)
        openRegistrationDialog(false);
}

void MainWindow::buildStatusBar()
{
    m_labelSnap = new QLabel(tr("Position X: 0.0   Y: 0.0"));
    m_labelRibbon = new QLabel(tr("Longueur : 280 mm"));
    m_labelTime = new QLabel(tr("Temps : 0 s"));
    m_labelSpires = new QLabel(tr("Spires : 0"));
    m_labelBights = new QLabel(tr("Bights : 0"));
    m_labelActiveRope = new QLabel(tr("Corde active : 1"));
    m_labelMode = new QLabel((m_view && m_view->isCrossingEditMode())
                                 ? tr("Mode : Insert (croisements)")
                                 : tr("Mode : Tra\u00E7age"));

    if (m_view && m_view->isCrossingEditMode())
        m_labelMode->setStyleSheet(QStringLiteral("QLabel { color: #b36b00; font-weight: 600; }"));

    statusBar()->addWidget(m_labelSnap);
    statusBar()->addWidget(m_labelRibbon);
    statusBar()->addWidget(m_labelTime);
    statusBar()->addWidget(m_labelSpires);
    statusBar()->addWidget(m_labelBights);
    statusBar()->addPermanentWidget(m_labelMode);
    statusBar()->addPermanentWidget(m_labelActiveRope);

    refreshActiveRopeUi();

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::refreshStatusBar);
    timer->start(1000);
}

QString MainWindow::findTranslationFile(const QString& languageCode) const
{
    const QString rel = QStringLiteral("translations/%1.qm").arg(languageCode);

    const QStringList candidates = {
        QDir(QCoreApplication::applicationDirPath()).filePath(rel),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../") + rel),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../../") + rel),
        QDir::current().filePath(rel),
        QDir::current().filePath(QStringLiteral("../") + rel),
        QDir::current().filePath(QStringLiteral("../../") + rel)
    };

    for (const QString& p : candidates)
    {
        if (QFileInfo::exists(p))
            return QDir::cleanPath(p);
    }

    return QString();
}

void MainWindow::applyLanguage(const QString& languageCode)
{
    if (!m_translator)
        m_translator = new QTranslator(this);

    qApp->removeTranslator(m_translator);

    bool installed = false;

    if (languageCode != QStringLiteral("fr"))
    {
        const QString qmPath = findTranslationFile(languageCode);

        if (!qmPath.isEmpty() && m_translator->load(qmPath))
        {
            qApp->installTranslator(m_translator);
            installed = true;
        }
    }

    m_currentLanguageCode = languageCode;
    applyLanguageTypography(languageCode);

    // reconstruction UI
    menuBar()->clear();
    buildMenusAndShortcuts();

    refreshUiTexts();

    // force repaint
    qApp->processEvents();
    update();

    if (statusBar())
    {
        if (installed || languageCode == QStringLiteral("fr"))
            statusBar()->showMessage(tr("Langue active : %1").arg(endonymForCode(languageCode)), 3000);
        else
            statusBar()->showMessage(tr("Traduction introuvable pour %1").arg(endonymForCode(languageCode)), 4000);
    }
}


void MainWindow::applyLanguageTypography(const QString& languageCode)
{
    if (!qApp)
        return;

    const QFont target = pickLanguageFont(languageCode, m_defaultAppFont);
    qApp->setFont(target);

    const bool rtl = (languageCode == QStringLiteral("ar") || languageCode == QStringLiteral("he"));
    qApp->setLayoutDirection(rtl ? Qt::RightToLeft : Qt::LeftToRight);
}
void MainWindow::refreshUiTexts()
{
    updateWindowTitle();
    refreshStatusBar();

    if (m_labelMode && m_view)
    {
        const bool insertMode = m_view->isCrossingEditMode();
        m_labelMode->setText(insertMode ? tr("Mode : Insert (croisements)")
                                        : tr("Mode : Tra\u00E7age"));
        m_labelMode->setStyleSheet(insertMode
                                       ? QStringLiteral("QLabel { color: #b36b00; font-weight: 600; }")
                                       : QString());
    }

    refreshActiveRopeUi();
}

void MainWindow::refreshActiveRopeUi()
{
    if (!m_model)
        return;

    const int rid = m_model->activeRopeId();
    const QColor c = m_model->activeRopeColor();

    if (m_labelActiveRope)
    {
        m_labelActiveRope->setText(
            tr("Corde active : %1 (%2)")
                .arg(rid + 1)
                .arg(c.name(QColor::HexRgb))
        );
        m_labelActiveRope->setStyleSheet(
            QStringLiteral("QLabel { color: %1; font-weight: 600; }")
                .arg(c.name(QColor::HexRgb))
        );
    }

    for (int i = 0; i < static_cast<int>(m_ropeActions.size()); ++i)
    {
        if (m_ropeActions[static_cast<std::size_t>(i)])
            m_ropeActions[static_cast<std::size_t>(i)]->setChecked(i == rid);
    }
}

void MainWindow::refreshStatusBar()
{
    if (!m_model)
        return;

    m_labelRibbon->setText(tr("Longueur : %1 mm").arg(m_model->ribbonLengthMM()));
    m_labelTime->setText(tr("Temps : %1 s").arg(m_model->designTimeSeconds()));

    const int points = static_cast<int>(m_model->points().size());
    const int spires = (points >= 2) ? ((points - 1) / 2) : 0;
    m_labelSpires->setText(tr("Spires: %1").arg(spires));

    const int bights = m_model->bightCount();
    m_labelBights->setText(tr("Bights: %1").arg(bights));

    if (m_labelSnap && m_labelSnap->text().isEmpty())
        m_labelSnap->setText(tr("Position X: 0.0   Y: 0.0"));

    refreshActiveRopeUi();
    updateFileActionsState();
}

void MainWindow::updateFileActionsState()
{
    if (!m_model)
        return;

    const bool canValidate = !m_currentFileValidated && m_model->canValidateAsLocked();

    if (m_actionSave)
        m_actionSave->setEnabled(!m_currentFileValidated);

    if (m_actionValidation)
        m_actionValidation->setEnabled(canValidate);

    if (m_actionCopy)
        m_actionCopy->setEnabled(m_currentFileValidated);
}

void MainWindow::setReadOnlyUiState(bool enabled)
{
    m_currentFileValidated = enabled;

    if (m_view)
    {
        if (enabled)
            m_view->enterTracingMode();

        m_view->setReadOnlyValidated(enabled);
    }

    if (m_actionSketchMode)
        m_actionSketchMode->setEnabled(!enabled);

    if (m_actionBreakSketch)
        m_actionBreakSketch->setEnabled(!enabled);

    if (m_actionTracingMode)
        m_actionTracingMode->setEnabled(!enabled);

    if (m_actionRotateRight45)
        m_actionRotateRight45->setEnabled(!enabled);

    if (m_actionInvertDirection)
        m_actionInvertDirection->setEnabled(!enabled);

    for (QAction* action : m_ropeActions)
    {
        if (action)
            action->setEnabled(!enabled);
    }

    updateFileActionsState();
}

void MainWindow::onSnapMoved(const QPointF& posMM)
{
    m_labelSnap->setText(
        tr("Position X: %1   Y: %2")
            .arg(posMM.x(), 0, 'f', 1)
            .arg(posMM.y(), 0, 'f', 1)
    );
}

void MainWindow::onNewFile()
{
    if (!m_model)
        return;

    const QMessageBox::StandardButton reply =
        QMessageBox::question(
            this,
            tr("Nouveau document"),
            tr("Creer un nouveau document ?\n\nLes modifications non sauvegardees seront perdues."),
            QMessageBox::Yes | QMessageBox::No
        );

    if (reply != QMessageBox::Yes)
        return;

    m_model->clear();
    m_model->initializeAuditForNewDocument(currentAuthorId());
    m_lastAuditCheckpointSeconds = m_model->designTimeSeconds();

    m_currentFilePath.clear();
    clearLastOpenedWorkspacePath();
    setReadOnlyUiState(false);
    updateWindowTitle();

    if (m_view)
    {
        m_view->syncFromModel();
        m_view->clearSketchOverlay();
    }

    refreshStatusBar();
    refreshActiveRopeUi();
}

void MainWindow::onSave()
{
    if (!m_model)
        return;

    if (m_currentFileValidated)
    {
        QMessageBox::information(
            this,
            tr("Fichier valide"),
            tr("Un fichier .lkv est verrouille. Utilisez d'abord 'Copie' pour creer un fichier .lkw editable."));
        return;
    }

    QString path = m_currentFilePath;

    if (path.isEmpty())
    {
        path = QFileDialog::getSaveFileName(
            this,
            tr("Enregistrer"),
            QDir(defaultWorkspaceDirectory()).filePath(QStringLiteral("Noname.lkw")),
            tr("LogiKnotting travail (*.lkw)")
        );

        if (path.isEmpty())
            return;
    }

    if (!path.endsWith(QStringLiteral(".lkw"), Qt::CaseInsensitive))
        path += QStringLiteral(".lkw");

    const std::int64_t nowSeconds = m_model->designTimeSeconds();
    std::int64_t sessionSeconds = nowSeconds - m_lastAuditCheckpointSeconds;
    if (sessionSeconds < 0)
        sessionSeconds = 0;

    m_model->appendAuditOnSave(currentAuthorId(), sessionSeconds);
    m_lastAuditCheckpointSeconds = nowSeconds;

    if (!m_model->saveToFile(path))
    {
        QMessageBox::warning(this, tr("Erreur"), tr("Echec de l'enregistrement."));
        return;
    }

    m_currentFilePath = path;
    saveLastOpenedWorkspacePath();
    setReadOnlyUiState(false);
    updateWindowTitle();

    statusBar()->showMessage(tr("Enregistre : %1").arg(path), 3000);
}

void MainWindow::onValidate()
{
    if (!m_model)
        return;

    if (m_currentFileValidated)
    {
        QMessageBox::information(this, tr("Validation"), tr("Ce fichier est deja valide (.lkv)."));
        return;
    }

    if (!m_model->canValidateAsLocked())
    {
        QMessageBox::warning(
            this,
            tr("Validation impossible"),
            tr("Toutes les cordes doivent etre fermees avant validation."));
        return;
    }

    QString suggestion = tr("Noname.lkv");
    if (!m_currentFilePath.isEmpty())
    {
        const QFileInfo fi(m_currentFilePath);
        suggestion = fi.absolutePath() + QDir::separator() + fi.completeBaseName() + QStringLiteral(".lkv");
    }
    else
    {
        suggestion = QDir(defaultWorkspaceDirectory()).filePath(QStringLiteral("Noname.lkv"));
    }

    QString path = QFileDialog::getSaveFileName(
        this,
        tr("Validation"),
        suggestion,
        tr("LogiKnotting valide (*.lkv)")
    );

    if (path.isEmpty())
        return;

    if (!path.endsWith(QStringLiteral(".lkv"), Qt::CaseInsensitive))
        path += QStringLiteral(".lkv");

    const std::int64_t nowSeconds = m_model->designTimeSeconds();
    std::int64_t sessionSeconds = nowSeconds - m_lastAuditCheckpointSeconds;
    if (sessionSeconds < 0)
        sessionSeconds = 0;

    m_model->appendAuditOnSave(currentAuthorId(), sessionSeconds);
    m_lastAuditCheckpointSeconds = nowSeconds;

    if (!m_model->saveToFile(path))
    {
        QMessageBox::warning(this, tr("Erreur"), tr("Echec de l'enregistrement du fichier valide."));
        return;
    }

    m_currentFilePath = path;
    saveLastOpenedWorkspacePath();
    setReadOnlyUiState(true);
    updateWindowTitle();

    refreshStatusBar();
    refreshActiveRopeUi();

    statusBar()->showMessage(tr("Valide : %1").arg(path), 3000);
}

void MainWindow::onCopyFromValidated()
{
    if (!m_model || !m_view)
        return;

    if (!m_currentFileValidated)
        return;

    m_view->copyModelGeometryToSketch();

    m_model->initializeAuditForNewDocument(currentAuthorId());
    m_lastAuditCheckpointSeconds = m_model->designTimeSeconds();

    m_currentFilePath.clear();
    clearLastOpenedWorkspacePath();
    setReadOnlyUiState(false);
    updateWindowTitle();

    refreshStatusBar();
    refreshActiveRopeUi();

    statusBar()->showMessage(tr("Copie creee. Mode Tracage actif, enregistrez sous Noname.lkw."), 4000);
}

QRectF MainWindow::computePrintSourceRectMM() const
{
    if (!m_model)
        return QRectF(0.0, 0.0, 280.0, 120.0);

    constexpr double kPaddingMM = 8.0;
    const double ribbonWidthMM = static_cast<double>(std::max(80, m_model->ribbonLengthMM()));
    const double ribbonOffsetMM = static_cast<double>(m_model->ribbonOffsetMM());

    bool hasData = false;
    double minY = 0.0;
    double maxY = 0.0;

    auto includeY = [&](double y)
    {
        if (!hasData)
        {
            minY = maxY = y;
            hasData = true;
            return;
        }

        minY = std::min(minY, y);
        maxY = std::max(maxY, y);
    };

    const auto& topo = m_model->topologySnapshot();
    for (const auto& rope : topo.ropes)
    {
        for (const auto& pt : rope.points)
            includeY(static_cast<double>(pt.y));
    }

    if (!hasData)
    {
        for (const auto& seg : m_model->segments())
        {
            includeY(seg.y1());
            includeY(seg.y2());
        }
    }

    if (!hasData)
    {
        for (const auto& p : m_model->points())
            includeY(p.y());
    }

    if (!hasData)
        return QRectF(ribbonOffsetMM - kPaddingMM, 0.0, ribbonWidthMM + 2.0 * kPaddingMM, 120.0);

    minY -= kPaddingMM;
    maxY += kPaddingMM;

    if ((maxY - minY) < 20.0)
    {
        const double cy = 0.5 * (maxY + minY);
        minY = cy - 10.0;
        maxY = cy + 10.0;
    }

    return QRectF(QPointF(ribbonOffsetMM - kPaddingMM, minY),
                  QPointF(ribbonOffsetMM + ribbonWidthMM + kPaddingMM, maxY)).normalized();
}

void MainWindow::drawCutAndGlueMarks(QPainter* painter,
                                     const QRectF& tileRectPx,
                                     bool hasLeftOverlap,
                                     bool hasTopOverlap,
                                     double overlapPx,
                                     double cutMarkPx) const
{
    if (!painter)
        return;

    QPen cutPen(QColor(15, 15, 15));
    cutPen.setWidthF(0.9);
    painter->setPen(cutPen);

    auto drawCorner = [&](double x, double y, double sx, double sy)
    {
        painter->drawLine(QPointF(x, y), QPointF(x + sx * cutMarkPx, y));
        painter->drawLine(QPointF(x, y), QPointF(x, y + sy * cutMarkPx));
    };

    drawCorner(tileRectPx.left(), tileRectPx.top(), -1.0, -1.0);
    drawCorner(tileRectPx.right(), tileRectPx.top(), +1.0, -1.0);
    drawCorner(tileRectPx.left(), tileRectPx.bottom(), -1.0, +1.0);
    drawCorner(tileRectPx.right(), tileRectPx.bottom(), +1.0, +1.0);

    QPen gluePen(QColor(160, 90, 10, 180));
    gluePen.setStyle(Qt::DashLine);
    gluePen.setWidthF(0.8);
    painter->setPen(gluePen);

    if (hasLeftOverlap && overlapPx > 0.5)
    {
        const double x = tileRectPx.left() + overlapPx;
        painter->drawLine(QPointF(x, tileRectPx.top()), QPointF(x, tileRectPx.bottom()));
    }

    if (hasTopOverlap && overlapPx > 0.5)
    {
        const double y = tileRectPx.top() + overlapPx;
        painter->drawLine(QPointF(tileRectPx.left(), y), QPointF(tileRectPx.right(), y));
    }
}

void MainWindow::renderPrintDocument(QPrinter* printer)
{
    if (!printer || !m_view || !m_view->scene())
        return;

    WorkspaceScene* workspaceScene = qobject_cast<WorkspaceScene*>(m_view->scene());
    const bool previousGridVisible = workspaceScene ? workspaceScene->isGridVisible() : true;
    const double previousStrokeWidthScale = workspaceScene ? workspaceScene->strokeWidthScale() : 1.0;
    const double previousPrintStrokeWidthMM = workspaceScene ? workspaceScene->printStrokeWidthMM() : 0.0;
    const bool hideGridForPrint = (workspaceScene && m_printHideGrid && previousGridVisible);
    if (hideGridForPrint)
        workspaceScene->setGridVisible(false);

    const auto restoreSceneState = [&]()
    {
        if (!workspaceScene)
            return;

        workspaceScene->setStrokeWidthScale(previousStrokeWidthScale);
        workspaceScene->setPrintStrokeWidthMM(previousPrintStrokeWidthMM);
        if (hideGridForPrint)
            workspaceScene->setGridVisible(previousGridVisible);
    };

    const QRectF sourceRectMM = computePrintSourceRectMM();
    if (sourceRectMM.width() <= 0.0 || sourceRectMM.height() <= 0.0)
    {
        restoreSceneState();
        return;
    }

    QRectF paintRectMM = printer->pageLayout().paintRect(QPageLayout::Millimeter);
    if (paintRectMM.width() <= 1.0 || paintRectMM.height() <= 1.0)
        paintRectMM = QRectF(10.0, 10.0, 190.0, 277.0);

    constexpr double kCutMarkMM = 4.0;
    constexpr double kInsetMM = 2.0;
    constexpr double kFooterMM = 6.0;
    const double glueOverlapMM = static_cast<double>(m_printOverlapMM);

    QRectF tileAreaMM = paintRectMM.adjusted(kCutMarkMM + kInsetMM,
                                             kCutMarkMM + kInsetMM,
                                             -(kCutMarkMM + kInsetMM),
                                             -(kCutMarkMM + kInsetMM + kFooterMM));

    if (tileAreaMM.width() < 20.0 || tileAreaMM.height() < 20.0)
        tileAreaMM = paintRectMM.adjusted(2.0, 2.0, -2.0, -2.0);

    const double tileWidthMM = tileAreaMM.width();
    const double tileHeightMM = tileAreaMM.height();

    const double sourceWidthMM = sourceRectMM.width();
    const double sourceHeightMM = sourceRectMM.height();
    const double printScale = std::min(1.0, tileHeightMM / sourceHeightMM);
    const double sourceTileWidthMM = tileWidthMM / printScale;
    const double sourceTileHeightMM = tileHeightMM / printScale;
    const double sourceOverlapMM = glueOverlapMM / printScale;

    const double stepXMM = std::max(1.0, sourceTileWidthMM - sourceOverlapMM);
    const double stepYMM = std::max(1.0, sourceTileHeightMM - sourceOverlapMM);

    const int cols = (sourceWidthMM <= sourceTileWidthMM)
        ? 1
        : static_cast<int>(std::ceil((sourceWidthMM - sourceTileWidthMM) / stepXMM)) + 1;
    const int rows = (sourceHeightMM <= sourceTileHeightMM)
        ? 1
        : static_cast<int>(std::ceil((sourceHeightMM - sourceTileHeightMM) / stepYMM)) + 1;

    const bool centerHorizontally = m_printCenterHorizontally && cols == 1;
    const bool centerVertically = m_printCenterVertically && rows == 1;

    if (workspaceScene)
    {
        workspaceScene->setStrokeWidthScale(1.0);
        workspaceScene->setPrintStrokeWidthMM(2.2);
    }

    QPainter painter(printer);
    if (!painter.isActive())
    {
        restoreSceneState();
        return;
    }

    painter.setRenderHint(QPainter::Antialiasing, true);

    const double pxPerMmX = static_cast<double>(printer->logicalDpiX()) / 25.4;
    const double pxPerMmY = static_cast<double>(printer->logicalDpiY()) / 25.4;

    auto mmRectToPx = [&](const QRectF& r)
    {
        return QRectF(r.x() * pxPerMmX,
                      r.y() * pxPerMmY,
                      r.width() * pxPerMmX,
                      r.height() * pxPerMmY);
    };

    int pageIndex = 0;
    const int totalPages = rows * cols;

    for (int row = 0; row < rows; ++row)
    {
        for (int col = 0; col < cols; ++col)
        {
            if (pageIndex > 0)
                printer->newPage();

            painter.fillRect(mmRectToPx(paintRectMM), Qt::white);

            double srcLeft = sourceRectMM.left() + static_cast<double>(col) * stepXMM;
            double srcTop = sourceRectMM.top() + static_cast<double>(row) * stepYMM;

            if (sourceWidthMM > sourceTileWidthMM && (srcLeft + sourceTileWidthMM) > sourceRectMM.right())
                srcLeft = sourceRectMM.right() - sourceTileWidthMM;
            if (sourceHeightMM > sourceTileHeightMM && (srcTop + sourceTileHeightMM) > sourceRectMM.bottom())
                srcTop = sourceRectMM.bottom() - sourceTileHeightMM;

            const double srcWidth = std::min(sourceTileWidthMM, sourceRectMM.right() - srcLeft);
            const double srcHeight = std::min(sourceTileHeightMM, sourceRectMM.bottom() - srcTop);
            if (srcWidth <= 0.0 || srcHeight <= 0.0)
            {
                ++pageIndex;
                continue;
            }

            const QRectF srcTileMM(srcLeft, srcTop, srcWidth, srcHeight);
            const double dstWidthMM = srcWidth * printScale;
            const double dstHeightMM = srcHeight * printScale;
            const double dstLeftMM = centerHorizontally
                ? tileAreaMM.left() + std::max(0.0, (tileAreaMM.width() - dstWidthMM) * 0.5)
                : tileAreaMM.left();
            const double dstTopMM = centerVertically
                ? tileAreaMM.top() + std::max(0.0, (tileAreaMM.height() - dstHeightMM) * 0.5)
                : tileAreaMM.top();

            const QRectF dstTileMM(dstLeftMM, dstTopMM, dstWidthMM, dstHeightMM);
            const QRectF dstTilePx = mmRectToPx(dstTileMM);

            m_view->scene()->render(&painter, dstTilePx, srcTileMM, Qt::IgnoreAspectRatio);

            const double overlapPxX = std::min(glueOverlapMM * pxPerMmX, dstTilePx.width());
            const double overlapPxY = std::min(glueOverlapMM * pxPerMmY, dstTilePx.height());

            if (col > 0 && overlapPxX > 0.5)
            {
                QRectF glueLeft = dstTilePx;
                glueLeft.setWidth(overlapPxX);
                painter.fillRect(glueLeft, QColor(255, 220, 120, 70));
            }

            if (row > 0 && overlapPxY > 0.5)
            {
                QRectF glueTop = dstTilePx;
                glueTop.setHeight(overlapPxY);
                painter.fillRect(glueTop, QColor(255, 220, 120, 70));
            }

            const double overlapGuidePx = std::min(overlapPxX, overlapPxY);
            drawCutAndGlueMarks(&painter,
                                dstTilePx,
                                col > 0,
                                row > 0,
                                overlapGuidePx,
                                std::min(pxPerMmX, pxPerMmY) * kCutMarkMM);

            QFont footerFont = painter.font();
            footerFont.setPointSizeF(std::max(7.0, footerFont.pointSizeF() - 1.0));
            painter.setFont(footerFont);
            painter.setPen(QColor(70, 70, 70));

            const QRectF footerRectPx = mmRectToPx(
                QRectF(paintRectMM.left(),
                       paintRectMM.bottom() - kFooterMM,
                       paintRectMM.width(),
                       kFooterMM));

            const QString footer = tr("Impression mosaique %1/%2 - tuile %3x%4 - recouvrement %5 mm")
                .arg(pageIndex + 1)
                .arg(totalPages)
                .arg(col + 1)
                .arg(row + 1)
                .arg(glueOverlapMM, 0, 'f', 0);

            painter.drawText(footerRectPx, Qt::AlignRight | Qt::AlignVCenter, footer);

            ++pageIndex;
        }
    }

    restoreSceneState();
}

void MainWindow::onPageSetup()
{
    if (!m_printer)
        return;

    QPageSetupDialog dialog(m_printer, this);
    dialog.setWindowTitle(tr("Mise en page"));

    if (dialog.exec() == QDialog::Accepted)
    {
        const QPageSize::PageSizeId selectedId = m_printer->pageLayout().pageSize().id();
        if (selectedId == QPageSize::A3)
            m_printPaperA3 = true;
        else if (selectedId == QPageSize::A4)
            m_printPaperA3 = false;
        else
            applyPrintProfileToPrinter();

        savePrintProfileSettings();
        buildMenusAndShortcuts();

        if (statusBar())
            statusBar()->showMessage(tr("Profil impression : %1, recouvrement %2 mm")
                                         .arg(printPaperProfileLabel())
                                         .arg(m_printOverlapMM),
                                     3000);
    }
}

void MainWindow::onPrintPreview()
{
    if (!m_printer || !m_view || !m_view->scene())
        return;

    QPrintPreviewDialog preview(m_printer, this);
    preview.setWindowTitle(tr("Apercu avant impression"));

    connect(&preview, &QPrintPreviewDialog::paintRequested,
            this, &MainWindow::renderPrintDocument);

    preview.exec();
}

void MainWindow::onPrint()
{
    if (!m_printer || !m_view || !m_view->scene())
        return;

    QPrintDialog dialog(m_printer, this);
    dialog.setWindowTitle(tr("Imprimer"));

    if (dialog.exec() != QDialog::Accepted)
        return;

    renderPrintDocument(m_printer);

    if (statusBar())
        statusBar()->showMessage(tr("Impression envoyee"), 2500);
}
void MainWindow::onOpen()
{
    if (!m_model)
        return;

    const QString initialPath = m_currentFilePath.isEmpty() ? lastOpenedWorkspacePath() : m_currentFilePath;
    const QString path = QFileDialog::getOpenFileName(
        this,
        tr("Ouvrir"),
        initialPath.isEmpty() ? defaultWorkspaceDirectory() : initialPath,
        tr("LogiKnotting (*.lkw *.lkv)")
    );

    if (path.isEmpty())
        return;

    if (!loadWorkspaceFromPath(path, true, true))
        return;
}

} // namespace LogiKnottingApp

// ============================================================
// End Of File
// ============================================================



















