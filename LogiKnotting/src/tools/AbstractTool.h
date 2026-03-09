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
// File        : src/tools/AbstractTool.h
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#ifndef ABSTRACTTOOL_H
#define ABSTRACTTOOL_H

#include <QObject>
#include <QPointF>

class WorkspaceModel;

// ============================================================
// AbstractTool
// ============================================================
// Classe de base polymorphe pour tous les outils utilisateur.
//
// Un outil :
// - interprète un geste utilisateur (clic, move, release)
// - agit éventuellement sur le modèle et/ou la scène
// - possède un cycle de vie explicite (activate / deactivate)
//
// Aucune logique métier n’est implémentée ici.
// ============================================================

class AbstractTool : public QObject
{
    Q_OBJECT

public:
    explicit AbstractTool(WorkspaceModel* model,
                          QObject* parent = nullptr);

    virtual ~AbstractTool();

    // --------------------------------------------------------
    // Cycle de vie
    // --------------------------------------------------------
    virtual void activate();
    virtual void deactivate();

    // --------------------------------------------------------
    // Événements souris (coordonnées scène)
    // --------------------------------------------------------
    virtual void mousePress(const QPointF& scenePos);
    virtual void mouseMove(const QPointF& scenePos);
    virtual void mouseRelease(const QPointF& scenePos);

protected:
    // Accès optionnel au modèle (peut être nullptr)
    WorkspaceModel* m_model = nullptr;
};

#endif

// ============================================================
// End Of File
// ============================================================
