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
// File        : src/ui/KnottingMainWindow.h
// Created     : 2026-03-06
// Updated     : 2026-03-11
// Description :
// ============================================================
#pragma once

#include <QMainWindow>
#include <QPointF>
#include <QFont>
#include <QRectF>

#include <cstdint>
#include <vector>

class QAction;
class QActionGroup;
class QCheckBox;
class QDialog;
class QLabel;
class QMediaPlayer;
class QPainter;
class QPrinter;
class QPushButton;
class QTextBrowser;
class QTranslator;
class QVideoWidget;
class WorkspaceView;

namespace Model {
class WorkspaceModel;
}

namespace LogiKnottingApp {

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
    void buildTutorialPanel();
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
    void refreshInteractionModeUi(bool announceStatus = false);
    void applyChromeStyle();
    void updateTutorialUi();
    void replayTutorialStep();
    void startTutorial(bool resetToBlankDocument);
    void advanceTutorial(int delta);
    void finishTutorial();
    void closeTutorial();
    void startStartupTutorialIfNeeded(bool restoredLastWorkspace);
    bool createBlankDocument(bool confirmUser);
    bool tutorialFirstRunCompleted() const;
    bool tutorialReplayOnStartup() const;
    void saveTutorialSettings(bool completed, bool replayOnStartup) const;

    void applyLanguage(const QString& languageCode);
    QString findTranslationFile(const QString& languageCode) const;
    void applyLanguageTypography(const QString& languageCode);

    // Actions
    void onNewFile();
    void onSave();
    void onOpen();
    void onValidate();
    void onCopyFromValidated();
    void onPageSetup();
    void onPrintPreview();
    void onPrint();
    void updateFileActionsState();
    void setReadOnlyUiState(bool enabled);
    void loadPrintProfileSettings();
    void savePrintProfileSettings() const;
    void applyPrintProfileToPrinter();
    QString printPaperProfileLabel() const;
    bool loadWorkspaceFromPath(const QString& path, bool showWarning, bool announceStatus);
    QString lastOpenedWorkspacePath() const;
    void saveLastOpenedWorkspacePath() const;
    void clearLastOpenedWorkspacePath() const;
    QString defaultWorkspaceDirectory() const;
    void showDefaultWorkspaceDirectoryDialog();

    // Impression
    QRectF computePrintSourceRectMM() const;
    void renderPrintDocument(QPrinter* printer);
    void drawCutAndGlueMarks(QPainter* painter,
                             const QRectF& tileRectPx,
                             bool hasLeftOverlap,
                             bool hasTopOverlap,
                             double overlapPx,
                             double cutMarkPx) const;

    // Signal recu depuis WorkspaceView
    void onSnapMoved(const QPointF& posMM);

private:
    QLabel* m_labelBights = nullptr;
    QLabel* m_labelSpires = nullptr;
    QLabel* m_labelActiveRope = nullptr;
    QLabel* m_labelMode = nullptr;
    QAction* m_actionSave = nullptr;
    QAction* m_actionValidation = nullptr;
    QAction* m_actionCopy = nullptr;
    QAction* m_actionPageSetup = nullptr;
    QAction* m_actionPrintPreview = nullptr;
    QAction* m_actionPrint = nullptr;
    QAction* m_actionSketchMode = nullptr;
    QAction* m_actionBreakSketch = nullptr;
    QAction* m_actionClearSketches = nullptr;
    QAction* m_actionTracingMode = nullptr;
    QAction* m_actionCrossingMode = nullptr;
    QAction* m_actionRotateRight90 = nullptr;
    QAction* m_actionRotateLeft90 = nullptr;
    QAction* m_actionFlipVertical = nullptr;
    QAction* m_actionFlipHorizontal = nullptr;
    QAction* m_actionZoomIn = nullptr;
    QAction* m_actionZoomOut = nullptr;
    QAction* m_actionZoomReset = nullptr;
    QAction* m_actionPlayAnimation = nullptr;
    QAction* m_actionTutorial = nullptr;
    QActionGroup* m_ropeActionGroup = nullptr;
    std::vector<QAction*> m_ropeActions;

    QString m_currentLanguageCode = QStringLiteral("fr");
    QTranslator* m_translator = nullptr;
    QFont m_defaultAppFont;

    WorkspaceView* m_view = nullptr;
    Model::WorkspaceModel* m_model = nullptr;
    QPrinter* m_printer = nullptr;
    bool m_printPaperA3 = false;
    int m_printOverlapMM = 8;
    bool m_printHideGrid = false;
    bool m_printCenterHorizontally = false;
    bool m_printCenterVertically = false;
    QString m_currentFilePath;
    bool m_currentFileValidated = false;
    std::int64_t m_lastAuditCheckpointSeconds = 0;
    QLabel* m_labelSnap = nullptr;
    QLabel* m_labelRibbon = nullptr;
    QLabel* m_labelTime = nullptr;
    QDialog* m_tutorialDock = nullptr;
    QLabel* m_tutorialStepLabel = nullptr;
    QLabel* m_tutorialTitleLabel = nullptr;
    QTextBrowser* m_tutorialBody = nullptr;
    QLabel* m_tutorialVideoLabel = nullptr;
    QVideoWidget* m_tutorialVideoWidget = nullptr;
    QLabel* m_tutorialCompletionPanel = nullptr;
    QPushButton* m_tutorialPreviousButton = nullptr;
    QPushButton* m_tutorialReplayButton = nullptr;
    QPushButton* m_tutorialNextButton = nullptr;
    QPushButton* m_tutorialFinishButton = nullptr;
    QCheckBox* m_tutorialReplayCheck = nullptr;
    QMediaPlayer* m_tutorialPlayer = nullptr;
    int m_tutorialStepIndex = 0;
    bool m_tutorialCompletedThisRun = false;
};

} // namespace LogiKnottingApp

// ============================================================
// End Of File
// ============================================================

