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
// File        : src/tools/RepereTool.h
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#ifndef REPERETOOL_H
#define REPERETOOL_H

#include "AbstractTool.h"

class QGraphicsScene;

// ============================================================
// RepereTool
// ============================================================
// Outil de CONTEXTE chargé de poser / retirer
// des points repères graphiques.
//
// Règles fondatrices :
// - hors séquence
// - hors WorkspaceModel
// - aucun impact topologique
// - action strictement visuelle et réversible
// ============================================================

class RepereTool : public AbstractTool
{
    Q_OBJECT

public:
    RepereTool(WorkspaceModel* model,
               QGraphicsScene* scene,
               QObject* parent = nullptr);

    // --------------------------------------------------------
    // Événement souris
    // --------------------------------------------------------
    // Toggle création / suppression d’un RepereItem.
    void mousePress(const QPointF& scenePos) override;

private:
    // Scène graphique cible (OBLIGATOIRE)
    QGraphicsScene* m_scene = nullptr;
};

#endif

// ============================================================
// End Of File
// ============================================================
