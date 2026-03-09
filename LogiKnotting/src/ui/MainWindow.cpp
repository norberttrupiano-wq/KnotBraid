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
// File        : src/ui/MainWindow.cpp
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#include "MainWindow.h"

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
#include <QStatusBar>
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
#include <QUrl>
#include <QUrlQuery>

#include "../model/workspacemodel.h"
#include "WorkspaceView.h"

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

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    m_translator = new QTranslator(this);
    m_defaultAppFont = qApp->font();

    buildUi();
    buildMenusAndShortcuts();
    buildStatusBar();

    if (m_model)
    {
        m_model->initializeAuditForNewDocument(currentAuthorId());
        m_lastAuditCheckpointSeconds = m_model->designTimeSeconds();
    }

    statusBar()->showMessage(tr("PrÃªt"));

    if (!ensureTrialAccess())
        QTimer::singleShot(0, qApp, &QCoreApplication::quit);
}

MainWindow::~MainWindow()
{
    delete m_view;
    delete m_model;
}

WorkspaceView* MainWindow::workspaceView() const
{
    return m_view;
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
    showMaximized();
}

void MainWindow::updateWindowTitle()
{
    const QString name =
        m_currentFilePath.isEmpty() ? tr("Noname.lkw") : QFileInfo(m_currentFilePath).fileName();

    setWindowTitle(tr("LogiKnotting â€” %1").arg(name));
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

    QMenu* fileMenu = menuBar()->addMenu(tr("&Fichier"));

    QAction* newAction = fileMenu->addAction(tr("Nouveau"));
    connect(newAction, &QAction::triggered, this, &MainWindow::onNewFile);

    QAction* actSave = fileMenu->addAction(tr("&Enregistrer"));
    actSave->setShortcut(QKeySequence::Save);
    connect(actSave, &QAction::triggered, this, &MainWindow::onSave);

    QAction* actOpen = fileMenu->addAction(tr("&Ouvrir"));
    actOpen->setShortcut(QKeySequence::Open);
    connect(actOpen, &QAction::triggered, this, &MainWindow::onOpen);

    fileMenu->addSeparator();
    QAction* filePropsAction = fileMenu->addAction(tr("Proprietes du fichier"));
    connect(filePropsAction, &QAction::triggered, this, &MainWindow::showFilePropertiesDialog);

    const QString viewMenuTitle = (m_currentLanguageCode == QStringLiteral("fr")) ? tr("Vue") : tr("View");
    QMenu* viewMenu = menuBar()->addMenu(viewMenuTitle);
    m_actionPlayAnimation = viewMenu->addAction(tr("Play Animation"));
    connect(m_actionPlayAnimation, &QAction::triggered, this, [this]() {
        if (m_view)
            m_view->startAnimation();
    });

    QMenu* sketchMenu = menuBar()->addMenu(tr("Esquisse"));

    QAction* actSketchMode = sketchMenu->addAction(tr("Mode Esquisse"));
    actSketchMode->setShortcut(QKeySequence("Ctrl+E"));
    connect(actSketchMode, &QAction::triggered, m_view, &WorkspaceView::enterSketchMode);

    QAction* actBreakSketch = sketchMenu->addAction(tr("Sectionner lâ€™esquisse"));
    actBreakSketch->setShortcut(QKeySequence(Qt::Key_Space));
    connect(actBreakSketch, &QAction::triggered, m_view, &WorkspaceView::breakSketch);

    sketchMenu->addSeparator();

    QAction* actTracingMode = sketchMenu->addAction(tr("Mode TraÃ§age"));
    actTracingMode->setShortcut(QKeySequence("Ctrl+B"));
    connect(actTracingMode, &QAction::triggered, m_view, &WorkspaceView::enterTracingMode);

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
            tr("CrÃ©er un nouveau document ?\n\nLes modifications non sauvegardÃ©es seront perdues."),
            QMessageBox::Yes | QMessageBox::No
        );

    if (reply != QMessageBox::Yes)
        return;

    m_model->clear();
    m_model->initializeAuditForNewDocument(currentAuthorId());
    m_lastAuditCheckpointSeconds = m_model->designTimeSeconds();

    m_currentFilePath.clear();
    updateWindowTitle();

    if (m_view)
        m_view->syncFromModel();

    refreshActiveRopeUi();
}

void MainWindow::onSave()
{
    if (!m_model)
        return;

    QString path = m_currentFilePath;

    if (path.isEmpty())
    {
        path = QFileDialog::getSaveFileName(
            this,
            tr("Enregistrer"),
            QString(),
            tr("LogiKnotting (*.lkv)")
        );

        if (path.isEmpty())
            return;
    }

    const std::int64_t nowSeconds = m_model->designTimeSeconds();
    std::int64_t sessionSeconds = nowSeconds - m_lastAuditCheckpointSeconds;
    if (sessionSeconds < 0)
        sessionSeconds = 0;

    m_model->appendAuditOnSave(currentAuthorId(), sessionSeconds);
    m_lastAuditCheckpointSeconds = nowSeconds;

    if (!m_model->saveToFile(path))
    {
        QMessageBox::warning(this, tr("Erreur"), tr("Ã‰chec de l'enregistrement."));
        return;
    }

    m_currentFilePath = path;
    updateWindowTitle();

    statusBar()->showMessage(tr("EnregistrÃ© : %1").arg(path), 3000);
}

void MainWindow::onOpen()
{
    if (!m_model)
        return;

    const QString path = QFileDialog::getOpenFileName(
        this,
        tr("Ouvrir"),
        QString(),
        tr("LogiKnotting (*.lk?)")
    );

    if (path.isEmpty())
        return;

    if (!m_model->loadFromFile(path))
    {
        QMessageBox::warning(
            this,
            tr("Erreur"),
            tr("Echec du chargement.\nFichier non conforme ou modifié manuellement."));
        return;
    }

    m_currentFilePath = path;
    m_lastAuditCheckpointSeconds = m_model->designTimeSeconds();
    updateWindowTitle();

    if (m_view && m_view->scene())
    {
        m_view->scene()->update();
        m_view->viewport()->update();
    }

    refreshStatusBar();
    refreshActiveRopeUi();

    statusBar()->showMessage(tr("Ouvert : %1").arg(path), 3000);
}

// ============================================================
// End Of File
// ============================================================


