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
#include <QCheckBox>
#include <QComboBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMediaPlayer>
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
#include <QVideoWidget>
#include <QTranslator>
#include <QCryptographicHash>
#include <QDate>
#include <QDesktopServices>
#include <QFormLayout>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QTextBrowser>
#include <QUrl>
#include <QUrlQuery>

#include <algorithm>
#include <cmath>

#include "../model/WorkspaceModel.h"
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

static QIcon makeSketchRotateIcon(bool clockwise)
{
    QPixmap pixmap(18, 18);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen guidePen(QColor(QStringLiteral("#8d8d8d")));
    guidePen.setWidthF(1.2);
    painter.setPen(guidePen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRectF(4.5, 4.5, 5.0, 5.0));

    QPen arrowPen(QColor(QStringLiteral("#2f2418")));
    arrowPen.setWidthF(1.6);
    arrowPen.setCapStyle(Qt::RoundCap);
    arrowPen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(arrowPen);

    if (clockwise)
    {
        painter.drawLine(QPointF(4.5, 3.5), QPointF(12.2, 3.5));
        painter.drawLine(QPointF(12.2, 3.5), QPointF(12.2, 11.2));
        painter.drawLine(QPointF(12.2, 11.2), QPointF(10.0, 9.4));
        painter.drawLine(QPointF(12.2, 11.2), QPointF(14.0, 9.0));
    }
    else
    {
        painter.drawLine(QPointF(13.5, 3.5), QPointF(5.8, 3.5));
        painter.drawLine(QPointF(5.8, 3.5), QPointF(5.8, 11.2));
        painter.drawLine(QPointF(5.8, 11.2), QPointF(3.9, 9.0));
        painter.drawLine(QPointF(5.8, 11.2), QPointF(8.0, 9.4));
    }

    return QIcon(pixmap);
}

static QIcon makeSketchFlipIcon(bool vertical)
{
    QPixmap pixmap(18, 18);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen axisPen(QColor(QStringLiteral("#8d8d8d")));
    axisPen.setWidthF(1.1);
    axisPen.setStyle(Qt::DashLine);
    painter.setPen(axisPen);

    QPen arrowPen(QColor(QStringLiteral("#2f2418")));
    arrowPen.setWidthF(1.6);
    arrowPen.setCapStyle(Qt::RoundCap);
    arrowPen.setJoinStyle(Qt::RoundJoin);

    if (vertical)
    {
        painter.drawLine(QPointF(3.5, 9.0), QPointF(14.5, 9.0));
        painter.setPen(arrowPen);
        painter.drawLine(QPointF(9.0, 7.2), QPointF(9.0, 3.6));
        painter.drawLine(QPointF(9.0, 3.6), QPointF(7.3, 5.3));
        painter.drawLine(QPointF(9.0, 3.6), QPointF(10.7, 5.3));
        painter.drawLine(QPointF(9.0, 10.8), QPointF(9.0, 14.4));
        painter.drawLine(QPointF(9.0, 14.4), QPointF(7.3, 12.7));
        painter.drawLine(QPointF(9.0, 14.4), QPointF(10.7, 12.7));
    }
    else
    {
        painter.drawLine(QPointF(9.0, 3.5), QPointF(9.0, 14.5));
        painter.setPen(arrowPen);
        painter.drawLine(QPointF(7.2, 9.0), QPointF(3.6, 9.0));
        painter.drawLine(QPointF(3.6, 9.0), QPointF(5.3, 7.3));
        painter.drawLine(QPointF(3.6, 9.0), QPointF(5.3, 10.7));
        painter.drawLine(QPointF(10.8, 9.0), QPointF(14.4, 9.0));
        painter.drawLine(QPointF(14.4, 9.0), QPointF(12.7, 7.3));
        painter.drawLine(QPointF(14.4, 9.0), QPointF(12.7, 10.7));
    }

    return QIcon(pixmap);
}

static QSettings appSettings()
{
    return QSettings(QSettings::NativeFormat,
                     QSettings::UserScope,
                     QStringLiteral("LogiKnotting"),
                     QStringLiteral("LogiKnotting"));
}

static QSettings shellSettings()
{
    return QSettings(QSettings::NativeFormat,
                     QSettings::UserScope,
                     QStringLiteral("KnotBraid"),
                     QStringLiteral("Shell"));
}

static QString uiLanguageSettingKey()
{
    return QStringLiteral("ui/language");
}

static QString normalizeConfiguredLanguageCode(const QString& configuredCode)
{
    QString normalized = configuredCode.trimmed();
    if (normalized.isEmpty())
        return QStringLiteral("fr");

    normalized.replace(QLatin1Char('-'), QLatin1Char('_'));

    if (normalized.compare(QStringLiteral("auto"), Qt::CaseInsensitive) == 0)
    {
        const QStringList systemLocales = QLocale::system().uiLanguages();
        for (const QString& locale : systemLocales)
        {
            QString candidate = locale.trimmed();
            candidate.replace(QLatin1Char('-'), QLatin1Char('_'));
            if (languageIndexForCode(candidate) >= 0)
                return candidate;

            const QString base = candidate.section(QLatin1Char('_'), 0, 0);
            if (languageIndexForCode(base) >= 0)
                return base;
        }

        return QStringLiteral("fr");
    }

    if (languageIndexForCode(normalized) >= 0)
        return normalized;

    const QString base = normalized.section(QLatin1Char('_'), 0, 0);
    if (languageIndexForCode(base) >= 0)
        return base;

    return QStringLiteral("fr");
}

static QString configuredUiLanguageCode()
{
    const QString shellCode = shellSettings().value(uiLanguageSettingKey()).toString().trimmed();
    if (!shellCode.isEmpty())
        return shellCode;

    const QString appCode = appSettings().value(uiLanguageSettingKey()).toString().trimmed();
    if (!appCode.isEmpty())
        return appCode;

    return QStringLiteral("fr");
}

static QString lastWorkspaceFileSettingKey()
{
    return QStringLiteral("ui/last_workspace_file");
}

static QString tutorialFirstRunSettingKey()
{
    return QStringLiteral("ui/tutorial/first_run_completed");
}

static QString tutorialReplaySettingKey()
{
    return QStringLiteral("ui/tutorial/replay_on_startup");
}

struct TutorialStep
{
    const char* title;
    const char* bodyHtml;
};

static const TutorialStep kTutorialSteps[] = {
    {
        QT_TRANSLATE_NOOP("MainWindow", "Bienvenue dans LogiKnotting"),
        QT_TRANSLATE_NOOP("MainWindow", "<p>Ce tutoriel presente le flux complet de <b>LogiKnotting</b> a partir d'un document vierge.</p>"
        "<p>Il suit l'ordre naturel de travail :</p>"
        "<p>1. preparer un ruban vide<br/>"
        "2. tracer une corde<br/>"
        "3. construire ou ajuster une esquisse<br/>"
        "4. corriger les croisements<br/>"
        "5. enregistrer, valider et imprimer</p>"
        "<p>Le tutoriel montre maintenant chaque geste en video, et tu peux revoir chaque etape autant de fois que necessaire.</p>")
    },
    {
        QT_TRANSLATE_NOOP("MainWindow", "Etape 1 - Le document de depart"),
        QT_TRANSLATE_NOOP("MainWindow", "<p>En <b>Mode Tracage</b>, tu travailles corde par corde sur un ruban millimetre.</p>"
        "<p>Repere les zones importantes :</p>"
        "<p>- la regle bleue en haut du ruban<br/>"
        "- la barre d'etat avec la position, la longueur, le temps, les spires et les ganses<br/>"
        "- le menu <b>Modes</b> pour passer de Tracage a Esquisses puis Croisements</p>"
        "<p>Le tutoriel commence volontairement sur un fichier vide pour montrer la logique append-only du trace.</p>")
    },
    {
        QT_TRANSLATE_NOOP("MainWindow", "Etape 2 - Tracer la premiere corde"),
        QT_TRANSLATE_NOOP("MainWindow", "<p>En tracage, pose les points avec le <b>clic gauche</b>.</p>"
        "<p>Utilise ensuite le clavier pour piloter le ruban :</p>"
        "<p>- <b>Fleche gauche / droite</b> : rotation du ruban par 1 mm<br/>"
        "- <b>Shift</b> : pas de 5 mm<br/>"
        "- <b>Ctrl</b> : pas de 10 mm</p>"
        "<p>Tu peux changer de corde avec <b>Ctrl+1</b> a <b>Ctrl+5</b>. Chaque corde garde sa propre couleur et sa propre suite de segments.</p>")
    },
    {
        QT_TRANSLATE_NOOP("MainWindow", "Etape 3 - Construire une esquisse"),
        QT_TRANSLATE_NOOP("MainWindow", "<p>Passe en <b>Mode Esquisses</b> pour preparer une geometrie temporaire, transformer une forme ou recopier un guide.</p>"
        "<p>En esquisse :</p>"
        "<p>- le premier clic pose une ancre<br/>"
        "- les clics suivants ajoutent des segments guides<br/>"
        "- <b>Espace</b> interrompt l'esquisse pour repartir ailleurs</p>"
        "<p>Les esquisses sont maintenant sauvegardees dans les fichiers <b>.lkw</b>, ce qui permet de reprendre un travail long sur plusieurs jours.</p>")
    },
    {
        QT_TRANSLATE_NOOP("MainWindow", "Etape 4 - Transformer l'esquisse"),
        QT_TRANSLATE_NOOP("MainWindow", "<p>Le menu <b>Modes</b> contient aussi les transformations d'esquisse :</p>"
        "<p>- pivoter a droite 90 degres<br/>"
        "- pivoter a gauche 90 degres<br/>"
        "- retourner verticalement<br/>"
        "- retourner horizontalement</p>"
        "<p>Utilise <b>Shift+glisser</b> pour selectionner puis deplacer une zone, <b>Ctrl+Shift+glisser</b> pour dupliquer une selection et <b>Suppr</b> pour supprimer la selection d'esquisse.</p>")
    },
    {
        QT_TRANSLATE_NOOP("MainWindow", "Etape 5 - Corriger les croisements"),
        QT_TRANSLATE_NOOP("MainWindow", "<p>Passe en <b>Mode Croisements</b> avec <b>Ctrl+C</b>.</p>"
        "<p>Dans ce mode :</p>"
        "<p>- <b>clic droit</b> sur un croisement : inverse le sur/dessous<br/>"
        "- <b>clic gauche</b> sur un segment : le prend comme source<br/>"
        "- <b>clic gauche</b> sur un autre segment comparable : recopie l'ordre des croisements</p>"
        "<p>Le mode Croisements est exclusif : on ne peut pas etre en Tracage ou en Esquisses en meme temps.</p>")
    },
    {
        QT_TRANSLATE_NOOP("MainWindow", "Etape 6 - Enregistrer et valider"),
        QT_TRANSLATE_NOOP("MainWindow", "<p>Travaille en <b>.lkw</b> tant que le modele n'est pas termine.</p>"
        "<p>Le fichier <b>.lkw</b> conserve :</p>"
        "<p>- les cordes tracees<br/>"
        "- les corrections de croisements<br/>"
        "- les esquisses en cours</p>"
        "<p>Quand le noeud est final, utilise <b>Validation</b> pour produire un <b>.lkv</b> verrouille. Si tu dois retravailler un fichier valide, utilise ensuite <b>Copie</b>.</p>")
    },
    {
        QT_TRANSLATE_NOOP("MainWindow", "Etape 7 - Imprimer le ruban"),
        QT_TRANSLATE_NOOP("MainWindow", "<p>Avant l'impression, ouvre l'<b>Apercu avant impression</b> pour verifier le cadrage, le nombre de pages et l'epaisseur du ruban.</p>"
        "<p>Le profil d'impression permet de regler :</p>"
        "<p>- le format papier A4 ou A3<br/>"
        "- le recouvrement<br/>"
        "- le centrage horizontal / vertical<br/>"
        "- l'affichage ou non de la grille</p>"
        "<p>L'objectif est d'obtenir un ruban lisible et proche du rendu ecran.</p>")
    },
    {
        QT_TRANSLATE_NOOP("MainWindow", "Etape 8 - Reprendre plus tard"),
        QT_TRANSLATE_NOOP("MainWindow", "<p>Quelques reperes utiles pour la suite :</p>"
        "<p>- <b>F1</b> ouvre le manuel depuis le shell KnotBraid<br/>"
        "- le dernier noeud charge est recharge au demarrage<br/>"
        "- la palette flottante du shell memorise sa position selon l'application</p>"
        "<p>Tu peux relancer ce tutoriel plus tard depuis <b>Parametres &gt; Tutoriel de demarrage</b>.</p>"
        "<p>Si tu coches l'option ci-dessous, il ne se relancera plus automatiquement a l'ouverture.</p>")
    }
};

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

static int tutorialFallbackStepCount()
{
    return static_cast<int>(sizeof(kTutorialSteps) / sizeof(kTutorialSteps[0]));
}

static QStringList tutorialAssetRootCandidates()
{
    const QString appDir = QCoreApplication::applicationDirPath();
    return {
        QDir(appDir).filePath(QStringLiteral("docs/tutorial")),
        QDir(appDir).filePath(QStringLiteral("docs/Tutorial")),
        QDir(appDir).filePath(QStringLiteral("docs/tutoriel")),
        QDir(appDir).filePath(QStringLiteral("docs/Tutoriel")),
        QDir(appDir).filePath(QStringLiteral("docs/Videos Tutoriel")),
        QDir(appDir).filePath(QStringLiteral("docs/Vid\u00E9os Tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../docs/tutorial")),
        QDir(appDir).filePath(QStringLiteral("../docs/Tutorial")),
        QDir(appDir).filePath(QStringLiteral("../docs/tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../docs/Tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../docs/Videos Tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../docs/Vid\u00E9os Tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../Docs/tutorial")),
        QDir(appDir).filePath(QStringLiteral("../Docs/Tutorial")),
        QDir(appDir).filePath(QStringLiteral("../Docs/tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../Docs/Tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../Docs/Videos Tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../Docs/Vid\u00E9os Tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../../docs/tutorial")),
        QDir(appDir).filePath(QStringLiteral("../../docs/Tutorial")),
        QDir(appDir).filePath(QStringLiteral("../../docs/tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../../docs/Tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../../docs/Videos Tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../../docs/Vid\u00E9os Tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../../Docs/tutorial")),
        QDir(appDir).filePath(QStringLiteral("../../Docs/Tutorial")),
        QDir(appDir).filePath(QStringLiteral("../../Docs/tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../../Docs/Tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../../Docs/Videos Tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../../Docs/Vid\u00E9os Tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../../../docs/tutorial")),
        QDir(appDir).filePath(QStringLiteral("../../../docs/Tutorial")),
        QDir(appDir).filePath(QStringLiteral("../../../docs/tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../../../docs/Tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../../../docs/Videos Tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../../../docs/Vid\u00E9os Tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../../../Docs/tutorial")),
        QDir(appDir).filePath(QStringLiteral("../../../Docs/Tutorial")),
        QDir(appDir).filePath(QStringLiteral("../../../Docs/tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../../../Docs/Tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../../../Docs/Videos Tutoriel")),
        QDir(appDir).filePath(QStringLiteral("../../../Docs/Vid\u00E9os Tutoriel")),
        QDir::current().filePath(QStringLiteral("docs/tutorial")),
        QDir::current().filePath(QStringLiteral("docs/Tutorial")),
        QDir::current().filePath(QStringLiteral("docs/tutoriel")),
        QDir::current().filePath(QStringLiteral("docs/Tutoriel")),
        QDir::current().filePath(QStringLiteral("docs/Videos Tutoriel")),
        QDir::current().filePath(QStringLiteral("docs/Vid\u00E9os Tutoriel")),
        QDir::current().filePath(QStringLiteral("Docs/tutorial")),
        QDir::current().filePath(QStringLiteral("Docs/Tutorial")),
        QDir::current().filePath(QStringLiteral("Docs/tutoriel")),
        QDir::current().filePath(QStringLiteral("Docs/Tutoriel")),
        QDir::current().filePath(QStringLiteral("Docs/Videos Tutoriel")),
        QDir::current().filePath(QStringLiteral("Docs/Vid\u00E9os Tutoriel"))
    };
}

static QString findTutorialAssetPath(const QString& fileName)
{
    if (fileName.trimmed().isEmpty())
        return QString();

    for (const QString& root : tutorialAssetRootCandidates())
    {
        const QString candidate = QDir(root).filePath(fileName);
        if (QFileInfo::exists(candidate))
            return QDir::cleanPath(candidate);
    }

    return QString();
}

static QString tutorialVideoNameForStep(int stepIndex)
{
    if (stepIndex < 0)
        return QString();

    return QStringLiteral("Etape %1.mp4").arg(stepIndex + 1);
}

static QString tutorialTextNameForStep(int stepIndex)
{
    if (stepIndex < 0)
        return QString();

    return QStringLiteral("Etape %1.txt").arg(stepIndex + 1);
}

static QString tutorialHtmlFromPlainText(QString text)
{
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    text = text.trimmed();
    if (text.isEmpty())
        return QStringLiteral("<p></p>");

    const QStringList paragraphs = text.split(QStringLiteral("\n\n"), Qt::SkipEmptyParts);
    QStringList htmlParagraphs;
    htmlParagraphs.reserve(paragraphs.size());
    for (QString paragraph : paragraphs)
    {
        paragraph = paragraph.trimmed().toHtmlEscaped();
        paragraph.replace(QLatin1Char('\n'), QStringLiteral("<br/>"));
        htmlParagraphs.push_back(QStringLiteral("<p>%1</p>").arg(paragraph));
    }

    return htmlParagraphs.join(QString());
}

static bool loadTutorialTextForStep(int stepIndex, QString* titleOut, QString* bodyHtmlOut)
{
    const QString path = findTutorialAssetPath(tutorialTextNameForStep(stepIndex));
    if (path.isEmpty())
        return false;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QString text = QString::fromUtf8(file.readAll());
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QLatin1Char('\r'), QLatin1Char('\n'));

    QString title;
    QString body = text.trimmed();
    QStringList lines = body.split(QLatin1Char('\n'));
    while (!lines.isEmpty() && lines.front().trimmed().isEmpty())
        lines.removeFirst();

    if (!lines.isEmpty())
    {
        const QString firstLine = lines.front().trimmed();
        if (firstLine.startsWith(QStringLiteral("# ")))
        {
            title = firstLine.mid(2).trimmed();
            lines.removeFirst();
        }
        else if (firstLine.startsWith(QStringLiteral("Titre:"), Qt::CaseInsensitive))
        {
            title = firstLine.mid(QStringLiteral("Titre:").size()).trimmed();
            lines.removeFirst();
        }
        else if (firstLine.startsWith(QStringLiteral("Title:"), Qt::CaseInsensitive))
        {
            title = firstLine.mid(QStringLiteral("Title:").size()).trimmed();
            lines.removeFirst();
        }
    }

    body = lines.join(QLatin1Char('\n')).trimmed();
    if (body.isEmpty() && !title.isEmpty())
        body = title;

    if (titleOut)
        *titleOut = title;
    if (bodyHtmlOut)
        *bodyHtmlOut = tutorialHtmlFromPlainText(body);
    return true;
}

static QString tutorialStepTitle(int stepIndex)
{
    QString externalTitle;
    if (loadTutorialTextForStep(stepIndex, &externalTitle, nullptr) && !externalTitle.trimmed().isEmpty())
        return externalTitle.trimmed();

    if (stepIndex >= 0 && stepIndex < tutorialFallbackStepCount())
        return QCoreApplication::translate("MainWindow", kTutorialSteps[stepIndex].title);

    return QObject::tr("Etape %1").arg(stepIndex + 1);
}

static QString tutorialStepBodyHtml(int stepIndex)
{
    QString externalHtml;
    if (loadTutorialTextForStep(stepIndex, nullptr, &externalHtml) && !externalHtml.trimmed().isEmpty())
        return externalHtml;

    if (stepIndex >= 0 && stepIndex < tutorialFallbackStepCount())
        return QCoreApplication::translate("MainWindow", kTutorialSteps[stepIndex].bodyHtml);

    return QStringLiteral("<p></p>");
}

static bool tutorialStepHasExternalAsset(int stepIndex)
{
    return !findTutorialAssetPath(tutorialVideoNameForStep(stepIndex)).isEmpty()
        || !findTutorialAssetPath(tutorialTextNameForStep(stepIndex)).isEmpty();
}

static int tutorialStepCount()
{
    int detectedCount = 0;
    bool foundExternalSequence = false;

    for (int stepIndex = 0; stepIndex < 128; ++stepIndex)
    {
        if (tutorialStepHasExternalAsset(stepIndex))
        {
            foundExternalSequence = true;
            detectedCount = stepIndex + 1;
            continue;
        }

        if (foundExternalSequence)
            break;
    }

    return foundExternalSequence ? detectedCount : tutorialFallbackStepCount();
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

    const QString startupLanguageCode = normalizeConfiguredLanguageCode(configuredUiLanguageCode());
    if (startupLanguageCode != QStringLiteral("fr"))
    {
        const QString qmPath = findTranslationFile(startupLanguageCode);
        if (!qmPath.isEmpty() && m_translator->load(qmPath))
            qApp->installTranslator(m_translator);
    }

    m_currentLanguageCode = startupLanguageCode;
    applyLanguageTypography(startupLanguageCode);

    loadPrintProfileSettings();

    m_printer = new QPrinter(QPrinter::HighResolution);
    m_printer->setFullPage(false);
    applyPrintProfileToPrinter();

    buildUi();
    buildTutorialPanel();
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
    else
        QTimer::singleShot(0, this, [this, restoredLastWorkspace]() { startStartupTutorialIfNeeded(restoredLastWorkspace); });
}

MainWindow::~MainWindow()
{
    delete m_view;
    delete m_model;
    delete m_printer;
}

void MainWindow::applyChromeStyle()
{
    if (menuBar())
    {
        menuBar()->setStyleSheet(QStringLiteral(
            "QMenuBar {"
            " background: #f6ede2;"
            " color: #2f2418;"
            " border-bottom: 1px solid #d7c2aa;"
            " padding: 2px 8px;"
            " }"
            "QMenuBar::item {"
            " color: #2f2418;"
            " background: transparent;"
            " padding: 6px 12px;"
            " border-radius: 6px;"
            " }"
            "QMenuBar::item:selected {"
            " background: #e5d5c1;"
            " color: #221a12;"
            " }"
            "QMenuBar::item:pressed {"
            " background: #dbc5ad;"
            " color: #221a12;"
            " }"
            "QMenu {"
            " background: #fffdfa;"
            " color: #2f2418;"
            " border: 1px solid #d7c2aa;"
            " }"
            "QMenu::item {"
            " color: #2f2418;"
            " padding: 6px 24px 6px 28px;"
            " }"
            "QMenu::item:selected {"
            " background: #efe1d0;"
            " color: #221a12;"
            " }"
            "QMenu::separator {"
            " height: 1px;"
            " background: #e3d3bf;"
            " margin: 5px 10px;"
            " }"));
    }

    if (statusBar())
    {
        statusBar()->setStyleSheet(QStringLiteral(
            "QStatusBar {"
            " background: #f6ede2;"
            " color: #2f2418;"
            " border-top: 1px solid #d7c2aa;"
            " }"
            "QStatusBar QLabel {"
            " color: #2f2418;"
            " }"
            "QStatusBar::item {"
            " border: none;"
            " }"));
    }
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
    {
        m_view->syncFromModel();
        if (validatedFile)
            m_view->clearSketchOverlay();
        else
            m_view->importSketchOverlayState(m_model->sketchOverlayState());
    }
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
            &WorkspaceView::interactionModeChanged,
            this,
            [this]()
            {
                refreshInteractionModeUi(true);
            });

    setCentralWidget(m_view);
    updateWindowTitle();
    if (parentWidget() == nullptr)
        showMaximized();
}

void MainWindow::buildTutorialPanel()
{
    if (m_tutorialDock)
        return;

    m_tutorialDock = new QDialog(this, Qt::Tool | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    m_tutorialDock->setObjectName(QStringLiteral("knottingTutorialWindow"));
    m_tutorialDock->setModal(false);
    m_tutorialDock->setWindowTitle(tr("Tutoriel LogiKnotting"));
    m_tutorialDock->setMinimumSize(900, 760);
    m_tutorialDock->resize(1040, 820);

    auto* layout = new QVBoxLayout(m_tutorialDock);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(10);

    m_tutorialStepLabel = new QLabel(m_tutorialDock);
    m_tutorialStepLabel->setObjectName(QStringLiteral("tutorialStepLabel"));

    m_tutorialTitleLabel = new QLabel(m_tutorialDock);
    m_tutorialTitleLabel->setWordWrap(true);
    m_tutorialTitleLabel->setObjectName(QStringLiteral("tutorialTitleLabel"));

    m_tutorialVideoLabel = new QLabel(m_tutorialDock);
    m_tutorialVideoLabel->setObjectName(QStringLiteral("tutorialVideoLabel"));

    m_tutorialVideoWidget = new QVideoWidget(m_tutorialDock);
    m_tutorialVideoWidget->setObjectName(QStringLiteral("tutorialVideoWidget"));
    m_tutorialVideoWidget->setMinimumHeight(460);
    m_tutorialVideoWidget->setAspectRatioMode(Qt::KeepAspectRatio);

    m_tutorialCompletionPanel = new QLabel(m_tutorialDock);
    m_tutorialCompletionPanel->setObjectName(QStringLiteral("tutorialCompletionPanel"));
    m_tutorialCompletionPanel->setMinimumHeight(460);
    m_tutorialCompletionPanel->setAlignment(Qt::AlignCenter);
    m_tutorialCompletionPanel->setWordWrap(true);
    m_tutorialCompletionPanel->hide();

    m_tutorialBody = new QTextBrowser(m_tutorialDock);
    m_tutorialBody->setObjectName(QStringLiteral("tutorialBody"));
    m_tutorialBody->setOpenExternalLinks(false);
    m_tutorialBody->setReadOnly(true);
    m_tutorialBody->setMinimumHeight(170);
    m_tutorialBody->setMaximumHeight(220);

    m_tutorialPlayer = new QMediaPlayer(this);
    m_tutorialPlayer->setVideoOutput(m_tutorialVideoWidget);

    m_tutorialReplayCheck = new QCheckBox(tr("Ne pas rejouer au demarrage"), m_tutorialDock);
    m_tutorialReplayCheck->setObjectName(QStringLiteral("tutorialReplayCheck"));

    auto* bottomLayout = new QHBoxLayout();
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(8);

    m_tutorialPreviousButton = new QPushButton(tr("Precedent"), m_tutorialDock);
    connect(m_tutorialPreviousButton, &QPushButton::clicked, this, [this]() { advanceTutorial(-1); });

    m_tutorialReplayButton = new QPushButton(tr("Revoir l'etape"), m_tutorialDock);
    connect(m_tutorialReplayButton, &QPushButton::clicked, this, &MainWindow::replayTutorialStep);

    m_tutorialNextButton = new QPushButton(tr("Suivant"), m_tutorialDock);
    connect(m_tutorialNextButton, &QPushButton::clicked, this, [this]() { advanceTutorial(1); });

    m_tutorialFinishButton = new QPushButton(tr("Terminer"), m_tutorialDock);
    connect(m_tutorialFinishButton, &QPushButton::clicked, this, &MainWindow::finishTutorial);

    auto* closeButton = new QPushButton(tr("Fermer"), m_tutorialDock);
    connect(closeButton, &QPushButton::clicked, this, &MainWindow::closeTutorial);

    bottomLayout->addWidget(m_tutorialReplayCheck);
    bottomLayout->addWidget(m_tutorialPreviousButton);
    bottomLayout->addWidget(m_tutorialReplayButton);
    bottomLayout->addWidget(m_tutorialNextButton);
    bottomLayout->addStretch(1);
    bottomLayout->addWidget(m_tutorialFinishButton);
    bottomLayout->addWidget(closeButton);

    layout->addWidget(m_tutorialStepLabel);
    layout->addWidget(m_tutorialTitleLabel);
    layout->addWidget(m_tutorialVideoLabel);
    layout->addWidget(m_tutorialVideoWidget, 1);
    layout->addWidget(m_tutorialCompletionPanel, 1);
    layout->addWidget(m_tutorialBody);
    layout->addLayout(bottomLayout);

    m_tutorialDock->setStyleSheet(QStringLiteral(
        "#knottingTutorialWindow { background: #fbf7f0; }"
        "#tutorialStepLabel { color: #8f5a27; font-size: 12px; font-weight: 700; }"
        "#tutorialTitleLabel { color: #1f1a16; font-size: 20px; font-weight: 700; }"
        "#tutorialBody { background: #fffdfa; border: 1px solid #d7c2aa; border-radius: 10px; color: #2f2418; padding: 6px; }"
        "#tutorialVideoLabel { color: #8f5a27; font-size: 12px; font-weight: 700; }"
        "#tutorialVideoWidget { background: #201810; border: 1px solid #d7c2aa; border-radius: 10px; }"
        "#tutorialCompletionPanel { background: #fffdfa; border: 1px solid #d7c2aa; border-radius: 10px; color: #2f2418; font-size: 24px; font-weight: 700; padding: 28px; }"
        "#tutorialReplayCheck { color: #2f2418; }"));

    m_tutorialDock->hide();
    updateTutorialUi();
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
    m_actionCrossingMode = nullptr;
    m_actionRotateRight90 = nullptr;
    m_actionRotateLeft90 = nullptr;
    m_actionFlipVertical = nullptr;
    m_actionFlipHorizontal = nullptr;
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

    QMenu* viewMenu = menuBar()->addMenu(tr("Vue"));

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
    QMenu* modesMenu = menuBar()->addMenu(tr("Modes"));
    QActionGroup* modeActionGroup = new QActionGroup(modesMenu);
    modeActionGroup->setExclusive(true);

    m_actionTracingMode = modesMenu->addAction(tr("Mode Tracage"));
    m_actionTracingMode->setCheckable(true);
    m_actionTracingMode->setShortcut(QKeySequence(QStringLiteral("Ctrl+B")));
    modeActionGroup->addAction(m_actionTracingMode);
    connect(m_actionTracingMode, &QAction::triggered, m_view, &WorkspaceView::enterTracingMode);

    modesMenu->addSeparator();

    m_actionSketchMode = modesMenu->addAction(tr("Mode Esquisses"));
    m_actionSketchMode->setCheckable(true);
    m_actionSketchMode->setShortcut(QKeySequence(QStringLiteral("Ctrl+E")));
    modeActionGroup->addAction(m_actionSketchMode);
    connect(m_actionSketchMode, &QAction::triggered, m_view, &WorkspaceView::enterSketchMode);

    m_actionRotateRight90 = modesMenu->addAction(makeSketchRotateIcon(true), tr("Pivoter a droite 90°"));
    connect(m_actionRotateRight90, &QAction::triggered, m_view, &WorkspaceView::rotateSelectionRight90);

    m_actionRotateLeft90 = modesMenu->addAction(makeSketchRotateIcon(false), tr("Pivoter a gauche 90°"));
    connect(m_actionRotateLeft90, &QAction::triggered, m_view, &WorkspaceView::rotateSelectionLeft90);

    m_actionFlipVertical = modesMenu->addAction(makeSketchFlipIcon(true), tr("Retourner verticalement"));
    connect(m_actionFlipVertical, &QAction::triggered, m_view, &WorkspaceView::flipSelectionVertically);

    m_actionFlipHorizontal = modesMenu->addAction(makeSketchFlipIcon(false), tr("Retourner horizontalement"));
    connect(m_actionFlipHorizontal, &QAction::triggered, m_view, &WorkspaceView::flipSelectionHorizontally);

    m_actionBreakSketch = modesMenu->addAction(tr("Interrompre l'esquisse"));
    m_actionBreakSketch->setShortcut(QKeySequence(Qt::Key_Space));
    connect(m_actionBreakSketch, &QAction::triggered, m_view, &WorkspaceView::breakSketch);

    m_actionClearSketches = modesMenu->addAction(tr("Effacer toutes les esquisses"));
    connect(m_actionClearSketches, &QAction::triggered, this, [this]() {
        if (!m_view)
            return;

        const auto answer = QMessageBox::question(
            this,
            tr("Effacer les esquisses"),
            tr("Effacer toutes les esquisses du document en cours ?"));
        if (answer != QMessageBox::Yes)
            return;

        m_view->clearSketchOverlay();
        refreshInteractionModeUi(false);
        refreshStatusBar();
        if (statusBar())
            statusBar()->showMessage(tr("Toutes les esquisses ont ete effacees."), 3000);
    });

    modesMenu->addSeparator();

    m_actionCrossingMode = modesMenu->addAction(tr("Mode Croisements"));
    m_actionCrossingMode->setCheckable(true);
    m_actionCrossingMode->setShortcut(QKeySequence(QStringLiteral("Ctrl+C")));
    modeActionGroup->addAction(m_actionCrossingMode);
    connect(m_actionCrossingMode, &QAction::triggered, m_view, &WorkspaceView::enterCrossingEditMode);

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
    applyChromeStyle();
}
void MainWindow::buildLanguageMenu()
{
    QMenu* settingsMenu = menuBar()->addMenu(tr("Param\u00E8tres"));

    QAction* languageAction = settingsMenu->addAction(tr("Langues..."));
    languageAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+L")));
    languageAction->setShortcutContext(Qt::ApplicationShortcut);
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

    m_actionTutorial = settingsMenu->addAction(tr("Tutoriel de demarrage"));
    connect(m_actionTutorial, &QAction::triggered, this, [this]() { startTutorial(true); });

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

bool MainWindow::tutorialFirstRunCompleted() const
{
    QSettings s = appSettings();
    return s.value(tutorialFirstRunSettingKey(), false).toBool();
}

bool MainWindow::tutorialReplayOnStartup() const
{
    QSettings s = appSettings();
    return s.value(tutorialReplaySettingKey(), true).toBool();
}

void MainWindow::saveTutorialSettings(bool completed, bool replayOnStartup) const
{
    QSettings s = appSettings();
    s.setValue(tutorialFirstRunSettingKey(), completed);
    s.setValue(tutorialReplaySettingKey(), replayOnStartup);
    s.sync();
}

void MainWindow::startStartupTutorialIfNeeded(bool restoredLastWorkspace)
{
    Q_UNUSED(restoredLastWorkspace);
    if (tutorialReplayOnStartup())
        startTutorial(false);
}

void MainWindow::startTutorial(bool resetToBlankDocument)
{
    Q_UNUSED(resetToBlankDocument);

    if (!m_tutorialDock)
        buildTutorialPanel();

    m_tutorialCompletedThisRun = false;
    m_tutorialStepIndex = 0;
    if (m_tutorialReplayCheck)
        m_tutorialReplayCheck->setChecked(!tutorialReplayOnStartup());

    updateTutorialUi();
    m_tutorialDock->show();
    m_tutorialDock->raise();
    if (statusBar())
        statusBar()->showMessage(tr("Tutoriel LogiKnotting ouvert."), 2500);
}

void MainWindow::advanceTutorial(int delta)
{
    const int stepCount = tutorialStepCount() + 1;
    m_tutorialStepIndex = std::clamp(m_tutorialStepIndex + delta, 0, stepCount - 1);
    updateTutorialUi();
}

void MainWindow::finishTutorial()
{
    m_tutorialCompletedThisRun = true;
    const bool replayOnStartup = m_tutorialReplayCheck ? !m_tutorialReplayCheck->isChecked() : false;
    saveTutorialSettings(true, replayOnStartup);
    closeTutorial();
    if (statusBar())
    {
        statusBar()->showMessage(
            replayOnStartup
                ? tr("Tutoriel termine. Il sera rejoue a l'ouverture.")
                : tr("Tutoriel termine. Il ne sera plus rejoue a l'ouverture."),
            4000);
    }
}

void MainWindow::closeTutorial()
{
    const bool replayOnStartup = m_tutorialReplayCheck ? !m_tutorialReplayCheck->isChecked() : tutorialReplayOnStartup();
    saveTutorialSettings(true, replayOnStartup);

    if (m_tutorialPlayer)
        m_tutorialPlayer->stop();
    if (m_tutorialDock)
        m_tutorialDock->hide();
}

void MainWindow::replayTutorialStep()
{
    if (!m_tutorialPlayer || m_tutorialPlayer->source().isEmpty())
        return;

    m_tutorialPlayer->setPosition(0);
    m_tutorialPlayer->play();
}

void MainWindow::updateTutorialUi()
{
    if (!m_tutorialDock || !m_tutorialStepLabel || !m_tutorialTitleLabel || !m_tutorialBody
        || !m_tutorialVideoLabel || !m_tutorialVideoWidget || !m_tutorialCompletionPanel || !m_tutorialPlayer
        || !m_tutorialPreviousButton || !m_tutorialReplayButton || !m_tutorialNextButton
        || !m_tutorialFinishButton || !m_tutorialReplayCheck)
        return;

    const int contentStepCount = tutorialStepCount();
    const int stepCount = contentStepCount + 1;
    const int clampedIndex = std::clamp(m_tutorialStepIndex, 0, stepCount - 1);
    const bool completionStep = (clampedIndex >= contentStepCount);

    m_tutorialDock->setWindowTitle(tr("Tutoriel LogiKnotting"));
    m_tutorialStepLabel->setText(tr("Etape %1/%2").arg(clampedIndex + 1).arg(stepCount));

    if (completionStep)
    {
        m_tutorialTitleLabel->setText(tr("Fin du tutoriel"));
        m_tutorialBody->setHtml(
            QStringLiteral("<p>%1</p><p>%2</p>")
                .arg(tr("Le tutoriel est termine."))
                .arg(tr("Vous pouvez fermer cette fenetre ou revenir a l'etape precedente pour revoir une demonstration.")));
        m_tutorialVideoLabel->setText(tr("Tutoriel termine"));
        m_tutorialVideoWidget->hide();
        m_tutorialCompletionPanel->setText(tr("Fin du tutoriel"));
        m_tutorialCompletionPanel->show();
        m_tutorialPlayer->stop();
        m_tutorialPlayer->setSource(QUrl());
        m_tutorialReplayButton->setEnabled(false);
    }
    else
    {
        m_tutorialTitleLabel->setText(tutorialStepTitle(clampedIndex));
        m_tutorialBody->setHtml(tutorialStepBodyHtml(clampedIndex));
        m_tutorialCompletionPanel->hide();

        const QString videoPath = findTutorialAssetPath(tutorialVideoNameForStep(clampedIndex));
        if (videoPath.isEmpty())
        {
            m_tutorialVideoLabel->setText(tr("Video de demonstration indisponible pour cette etape"));
            m_tutorialVideoWidget->hide();
            m_tutorialPlayer->stop();
            m_tutorialPlayer->setSource(QUrl());
            m_tutorialReplayButton->setEnabled(false);
        }
        else
        {
            m_tutorialVideoLabel->setText(tr("Video de demonstration - lecture simple"));
            m_tutorialVideoWidget->show();

            const QUrl sourceUrl = QUrl::fromLocalFile(videoPath);
            if (m_tutorialPlayer->source() != sourceUrl)
                m_tutorialPlayer->setSource(sourceUrl);
            m_tutorialPlayer->setPosition(0);
            m_tutorialPlayer->play();
            m_tutorialReplayButton->setEnabled(true);
        }
    }

    m_tutorialPreviousButton->setEnabled(clampedIndex > 0);
    m_tutorialReplayButton->setText(tr("Revoir l'etape"));
    m_tutorialNextButton->setText(completionStep ? tr("Fin du tutoriel") : tr("Suivant"));
    m_tutorialNextButton->setEnabled(!completionStep);
    m_tutorialNextButton->setVisible(true);
    m_tutorialFinishButton->setVisible(false);
    m_tutorialReplayCheck->setText(tr("Ne pas rejouer au demarrage"));
    m_tutorialReplayCheck->setVisible(true);
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
    m_labelBights = new QLabel(tr("Ganses : 0"));
    m_labelActiveRope = new QLabel(tr("Corde active : 1"));
    m_labelMode = new QLabel(tr("Mode : Tracage"));

    statusBar()->addWidget(m_labelSnap);
    statusBar()->addWidget(m_labelRibbon);
    statusBar()->addWidget(m_labelTime);
    statusBar()->addWidget(m_labelSpires);
    statusBar()->addWidget(m_labelBights);
    statusBar()->addPermanentWidget(m_labelMode);
    statusBar()->addPermanentWidget(m_labelActiveRope);

    refreshActiveRopeUi();
    refreshInteractionModeUi();

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::refreshStatusBar);
    timer->start(1000);
    applyChromeStyle();
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
    const QString normalizedLanguageCode = normalizeConfiguredLanguageCode(languageCode);

    QSettings shell = shellSettings();
    shell.setValue(uiLanguageSettingKey(), normalizedLanguageCode);
    shell.sync();

    QSettings app = appSettings();
    app.setValue(uiLanguageSettingKey(), normalizedLanguageCode);
    app.sync();

    if (!m_translator)
        m_translator = new QTranslator(this);

    qApp->removeTranslator(m_translator);

    bool installed = false;

    if (normalizedLanguageCode != QStringLiteral("fr"))
    {
        const QString qmPath = findTranslationFile(normalizedLanguageCode);

        if (!qmPath.isEmpty() && m_translator->load(qmPath))
        {
            qApp->installTranslator(m_translator);
            installed = true;
        }
    }

    m_currentLanguageCode = normalizedLanguageCode;
    applyLanguageTypography(normalizedLanguageCode);

    // reconstruction UI
    menuBar()->clear();
    buildMenusAndShortcuts();

    refreshUiTexts();

    // force repaint
    qApp->processEvents();
    update();

    if (statusBar())
    {
        if (installed || normalizedLanguageCode == QStringLiteral("fr"))
            statusBar()->showMessage(tr("Langue active : %1").arg(endonymForCode(normalizedLanguageCode)), 3000);
        else
            statusBar()->showMessage(tr("Traduction introuvable pour %1").arg(endonymForCode(normalizedLanguageCode)), 4000);
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
    refreshInteractionModeUi();
    refreshActiveRopeUi();
    updateTutorialUi();
}

void MainWindow::refreshInteractionModeUi(bool announceStatus)
{
    if (!m_view)
        return;

    const bool crossingMode = m_view->isCrossingEditMode();
    const bool sketchMode = m_view->isSketchMode();

    QString labelText = tr("Mode : Tracage");
    QString labelStyle;
    QString statusText;

    if (crossingMode)
    {
        labelText = tr("Mode : Croisements");
        labelStyle = QStringLiteral("QLabel { color: #b36b00; font-weight: 600; }");
        statusText = tr("Mode Croisements actif : clic droit pour inverser, clic gauche pour copier l'ordre des croisements d'un segment vers un autre.");
    }
    else if (sketchMode)
    {
        labelText = tr("Mode : Esquisse");
        labelStyle = QStringLiteral("QLabel { color: #2c6f44; font-weight: 600; }");
        statusText = tr("Mode Esquisse actif : tracer, transformer ou interrompre l'esquisse.");
    }
    else
    {
        statusText = tr("Mode Tracage actif.");
    }

    if (m_labelMode)
    {
        m_labelMode->setText(labelText);
        m_labelMode->setStyleSheet(labelStyle);
    }

    if (m_actionTracingMode)
        m_actionTracingMode->setChecked(!crossingMode && !sketchMode);
    if (m_actionSketchMode)
        m_actionSketchMode->setChecked(sketchMode);
    if (m_actionCrossingMode)
        m_actionCrossingMode->setChecked(crossingMode);

    const bool sketchCommandsEnabled = sketchMode && !m_currentFileValidated;
    if (m_actionRotateRight90)
        m_actionRotateRight90->setEnabled(sketchCommandsEnabled);
    if (m_actionRotateLeft90)
        m_actionRotateLeft90->setEnabled(sketchCommandsEnabled);
    if (m_actionFlipVertical)
        m_actionFlipVertical->setEnabled(sketchCommandsEnabled);
    if (m_actionFlipHorizontal)
        m_actionFlipHorizontal->setEnabled(sketchCommandsEnabled);
    if (m_actionBreakSketch)
        m_actionBreakSketch->setEnabled(sketchCommandsEnabled);
    if (m_actionClearSketches)
        m_actionClearSketches->setEnabled(sketchCommandsEnabled);

    if (announceStatus && statusBar())
        statusBar()->showMessage(statusText, 2200);
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
        m_labelBights->setText(tr("Ganses : %1").arg(bights));

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
    if (m_actionClearSketches)
        m_actionClearSketches->setEnabled(!enabled);

    if (m_actionTracingMode)
        m_actionTracingMode->setEnabled(!enabled);

    if (m_actionCrossingMode)
        m_actionCrossingMode->setEnabled(!enabled);

    if (m_actionRotateRight90)
        m_actionRotateRight90->setEnabled(!enabled);

    if (m_actionRotateLeft90)
        m_actionRotateLeft90->setEnabled(!enabled);

    if (m_actionFlipVertical)
        m_actionFlipVertical->setEnabled(!enabled);

    if (m_actionFlipHorizontal)
        m_actionFlipHorizontal->setEnabled(!enabled);

    for (QAction* action : m_ropeActions)
    {
        if (action)
            action->setEnabled(!enabled);
    }

    updateFileActionsState();
    refreshInteractionModeUi();
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
    createBlankDocument(true);
}

bool MainWindow::createBlankDocument(bool confirmUser)
{
    if (!m_model)
        return false;

    if (confirmUser)
    {
        const QMessageBox::StandardButton reply =
            QMessageBox::question(
                this,
                tr("Nouveau document"),
                tr("Creer un nouveau document ?\n\nLes modifications non sauvegardees seront perdues."),
                QMessageBox::Yes | QMessageBox::No
            );

        if (reply != QMessageBox::Yes)
            return false;
    }

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
    refreshInteractionModeUi();
    return true;
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

    if (m_view)
        m_model->setSketchOverlayState(m_view->exportSketchOverlayState());
    else
        m_model->clearSketchOverlayState();

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



















