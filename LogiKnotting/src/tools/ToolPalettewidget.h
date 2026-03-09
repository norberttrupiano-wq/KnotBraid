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
// File        : src/tools/ToolPalettewidget.h
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#ifndef TOOLPALETTEWIDGET_H
#define TOOLPALETTEWIDGET_H

#include <QWidget>

class QToolButton;
class QHideEvent;
class QEvent;

// ============================================================
// ToolPaletteWidget
// ============================================================
// Palette flottante éphémère :
// - émet des intentions
// - ne conserve aucun état
// - disparaît immédiatement après interaction
// ============================================================

class ToolPaletteWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ToolPaletteWidget(QWidget *parent = nullptr);

    // Affichage optionnel à la position du curseur
    void showAtCursor();

signals:
    // Sélection d’outil (TOOL_* défini dans WorkspaceView)
    void toolSelected(int toolId);

    // Sélection de choix (déplacement règle, mode plan/cylindre)
    void choiceSelected(int choiceId);

    // Émis systématiquement lors de hide()
    void paletteHidden();

    // Actions ponctuelles
    void openRequested();
    void saveRequested();

protected:
    // Interception clics hors palette → hide immédiat
    bool eventFilter(QObject *watched, QEvent *event) override;

    // Hook Qt hide()
    void hideEvent(QHideEvent *event) override;

private:
    // Construction UI
    void createButtons();

    // Masquage interne centralisé
    void hidePalette();

private:
    // --------------------------------------------------------
    // Boutons — outils
    // --------------------------------------------------------
    QToolButton *m_btnPoint        = nullptr; // Point Bleu
    QToolButton *m_btnErase       = nullptr; // Gomme
    QToolButton *m_btnPointRepere = nullptr; // Point Repère
    QToolButton *m_btnPointInit   = nullptr; // Point Init

    // --------------------------------------------------------
    // Boutons — modes / choix
    // --------------------------------------------------------
    QToolButton *m_btnRegleVerte = nullptr;   // Déplacement règle verte
    QToolButton *m_btnCylindre   = nullptr;   // Mode Cylindre

    // --------------------------------------------------------
    // Boutons — actions
    // --------------------------------------------------------
    QToolButton *m_btnOpen = nullptr;
    QToolButton *m_btnSave = nullptr;
};

#endif

// ============================================================
// End Of File
// ============================================================
