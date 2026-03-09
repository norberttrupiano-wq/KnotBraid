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
// File        : src/tools/RepereTool.cpp
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#include "RepereTool.h"
#include "../items/RepereItem.h"

#include <QGraphicsScene>
#include <QGraphicsItem>

// ============================================================
// CONSTRUCTEUR
// ============================================================

RepereTool::RepereTool(WorkspaceModel* model,
                       QGraphicsScene* scene,
                       QObject* parent)
    : AbstractTool(model, parent)
    , m_scene(scene)
{
    // Le modèle est volontairement non utilisé.
}

// ============================================================
// ÉVÉNEMENT SOURIS — CLIC
// ============================================================

void RepereTool::mousePress(const QPointF& scenePos)
{
    if (!m_scene)
        return;

    const QList<QGraphicsItem*> itemsAtPos = m_scene->items(scenePos);

    // Toggle repère
    for (QGraphicsItem* item : itemsAtPos)
    {
        if (auto* repere = dynamic_cast<RepereItem*>(item))
        {
            if (repere->gridPosition() == scenePos)
            {
                m_scene->removeItem(repere);
                delete repere;
                return;
            }
        }
    }

    // Création d’un repère volatile
    m_scene->addItem(new RepereItem(scenePos));
}

// ============================================================
// End Of File
// ============================================================
