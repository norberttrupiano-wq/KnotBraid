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
// File        : src/ui/MainWindow.h
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#pragma once

#include <QMainWindow>
#include <QPointF>
#include <QFont>

#include <cstdint>
#include <vector>

class QAction;
class QActionGroup;
class QLabel;
class QTranslator;
class WorkspaceView;

namespace Model {
class WorkspaceModel;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    void updateWindowTitle();
    WorkspaceView* workspaceView() const;

private:
    // Construction
    void buildUi();
    void buildMenusAndShortcuts();
    void buildStatusBar();
    void buildLanguageMenu();
    void openLanguageDialog();
    bool openRegistrationDialog(bool trialExpired);
    bool ensureTrialAccess();
    QString currentRegisteredEmail() const;
    QString currentAuthorId() const;
    void showFilePropertiesDialog();
    void showHelpDialog();
    void showAboutDialog();

    void refreshStatusBar();
    void refreshActiveRopeUi();
    void refreshUiTexts();

    void applyLanguage(const QString& languageCode);
    QString findTranslationFile(const QString& languageCode) const;
    void applyLanguageTypography(const QString& languageCode);

    // Actions
    void onNewFile();
    void onSave();
    void onOpen();

    // Signal reÃ§u depuis WorkspaceView
    void onSnapMoved(const QPointF& posMM);

private:
    QLabel* m_labelBights = nullptr;
    QLabel* m_labelSpires = nullptr;
    QLabel* m_labelActiveRope = nullptr;
    QLabel* m_labelMode = nullptr;
    QAction* m_actionPlayAnimation = nullptr;
    QActionGroup* m_ropeActionGroup = nullptr;
    std::vector<QAction*> m_ropeActions;

    QString m_currentLanguageCode = QStringLiteral("fr");
    QTranslator* m_translator = nullptr;
    QFont m_defaultAppFont;

    WorkspaceView* m_view = nullptr;
    Model::WorkspaceModel* m_model = nullptr;
    QString m_currentFilePath;
    std::int64_t m_lastAuditCheckpointSeconds = 0;
    QLabel* m_labelSnap = nullptr;
    QLabel* m_labelRibbon = nullptr;
    QLabel* m_labelTime = nullptr;
};

// ============================================================
// End Of File
// ============================================================
