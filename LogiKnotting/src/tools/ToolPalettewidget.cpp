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
// File        : src/tools/ToolPalettewidget.cpp
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#include "ToolPaletteWidget.h"
#include "../WorkspaceView.h"

#include <QToolButton>
#include <QGridLayout>
#include <QApplication>
#include <QCursor>
#include <QEvent>
#include <QMouseEvent>
#include <QHideEvent>
#include <QIcon>

// ============================================================
// CONSTRUCTEUR
// ============================================================

ToolPaletteWidget::ToolPaletteWidget(QWidget* parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint)
{
    setAttribute(Qt::WA_DeleteOnClose, false);
    setFocusPolicy(Qt::StrongFocus);

    createButtons();

    qApp->installEventFilter(this);
}

// ============================================================
// CONSTRUCTION DE L’INTERFACE
// ============================================================

void ToolPaletteWidget::createButtons()
{
    auto* layout = new QGridLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setHorizontalSpacing(6);
    layout->setVerticalSpacing(6);

    auto mkBtn = [this](const QString& icon, const QString& tip) {
        auto* b = new QToolButton(this);
        b->setIcon(QIcon(icon));
        b->setToolTip(tip);
        b->setAutoRaise(true);
        return b;
    };

    // Ligne 0 — Points
    m_btnPointInit   = mkBtn(":/icons/point01.png", tr("Point Init"));
    m_btnPointRepere = mkBtn(":/icons/repere.png",  tr("Point Repère"));
    layout->addWidget(m_btnPointInit,   0, 0);
    layout->addWidget(m_btnPointRepere, 0, 1);

    connect(m_btnPointInit, &QToolButton::clicked, this, [this]() {
        emit toolSelected(0);
        hidePalette();
    });

    connect(m_btnPointRepere, &QToolButton::clicked, this, [this]() {
        emit toolSelected(1);
        hidePalette();
    });

    // Ligne 1 — Actions réelles
    m_btnPoint = mkBtn(":/icons/point.png", tr("Point Bleu"));
    m_btnErase = mkBtn(":/icons/gomme.png", tr("Gomme"));
    layout->addWidget(m_btnPoint, 1, 0);
    layout->addWidget(m_btnErase, 1, 1);

    connect(m_btnPoint, &QToolButton::clicked, this, [this]() {
        emit toolSelected(2);
        hidePalette();
    });

    connect(m_btnErase, &QToolButton::clicked, this, [this]() {
        emit toolSelected(3);
        hidePalette();
    });

    // Ligne 2 — Modes
    m_btnRegleVerte = mkBtn(":/icons/fleche.png",  tr("Déplacement Règle verte"));
    m_btnCylindre  = mkBtn(":/icons/bascule.png", tr("Mode Plan ou Cylindre"));
    layout->addWidget(m_btnRegleVerte, 2, 0);
    layout->addWidget(m_btnCylindre,  2, 1);

    connect(m_btnRegleVerte, &QToolButton::clicked, this, [this]() {
        emit choiceSelected(CHOICE_MOVE_REGLE_VERTE);
        hidePalette();
    });

    connect(m_btnCylindre, &QToolButton::clicked, this, [this]() {
        emit toolSelected(-1);
        hidePalette();
    });

    // Ligne 3 — Fichiers
    m_btnOpen = mkBtn(":/icons/Open.png", tr("Ouvrir"));
    m_btnSave = mkBtn(":/icons/Save.png", tr("Sauver"));
    layout->addWidget(m_btnOpen, 3, 0);
    layout->addWidget(m_btnSave, 3, 1);

    connect(m_btnOpen, &QToolButton::clicked, this, [this]() {
        emit openRequested();
        hidePalette();
    });

    connect(m_btnSave, &QToolButton::clicked, this, [this]() {
        emit saveRequested();
        hidePalette();
    });
}

// ============================================================
// AFFICHAGE
// ============================================================

void ToolPaletteWidget::showAtCursor()
{
    move(QCursor::pos());
    show();
    raise();
    activateWindow();
}

// ============================================================
// FILTRE D’ÉVÉNEMENTS GLOBAL
// ============================================================

bool ToolPaletteWidget::eventFilter(QObject*, QEvent* event)
{
    if (!isVisible())
        return false;

    if (event->type() == QEvent::MouseButtonPress)
    {
        auto* me = static_cast<QMouseEvent*>(event);
        const QPoint globalPos = me->globalPosition().toPoint();

        if (!rect().contains(mapFromGlobal(globalPos)))
            hidePalette();
    }

    return false;
}

// ============================================================
// HOOK QT : HIDE
// ============================================================

void ToolPaletteWidget::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    emit paletteHidden();
}

// ============================================================
// API INTERNE
// ============================================================

void ToolPaletteWidget::hidePalette()
{
    hide();
}

// ============================================================
// End Of File
// ============================================================
