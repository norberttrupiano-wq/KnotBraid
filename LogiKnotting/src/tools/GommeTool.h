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
// File        : src/tools/GommeTool.h
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#ifndef GOMMETOOL_H
#define GOMMETOOL_H

#include <QObject>
#include <QPointF>

class WorkspaceModel;

// ============================================================================
// GommeTool
// ============================================================================
// Outil utilisateur NON DESTRUCTIF.
//
// Règles fondatrices :
// - ne modifie jamais la séquence append-only
// - ne supprime aucun point ni segment
// - n’agit que sur le CONTEXTE VISUEL
//
// Implémentation fonctionnelle à venir.
// ============================================================================

class GommeTool : public QObject
{
    Q_OBJECT

public:
    explicit GommeTool(WorkspaceModel *model,
                       QObject *parent = nullptr);

    ~GommeTool();

    // --------------------------------------------------------
    // Cycle de vie
    // --------------------------------------------------------
    void activate();
    void deactivate();

    // --------------------------------------------------------
    // Événements souris (coordonnées scène)
    // --------------------------------------------------------
    void mousePress(const QPointF &pos);
    void mouseMove(const QPointF &pos);
    void mouseRelease(const QPointF &pos);

private:
    WorkspaceModel *m_model;
    bool m_active;
};

#endif

// ============================================================
// End Of File
// ============================================================
