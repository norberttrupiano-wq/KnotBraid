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
// File        : src/tools/AbstractTool.cpp
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#include "AbstractTool.h"
#include "../workspace/WorkspaceModel.h"

// ============================================================
// CONSTRUCTEUR / DESTRUCTEUR
// ============================================================

AbstractTool::AbstractTool(WorkspaceModel* model, QObject* parent)
    : QObject(parent)
    , m_model(model)
{
    // Modèle injecté à la construction (responsabilité du caller)
}

AbstractTool::~AbstractTool() = default;

// ============================================================
// CYCLE DE VIE DE L’OUTIL
// ============================================================

void AbstractTool::activate()
{
    // Par défaut : rien
}

void AbstractTool::deactivate()
{
    // Par défaut : rien
}

// ============================================================
// ÉVÉNEMENTS SOURIS (API GÉNÉRIQUE)
// ============================================================

void AbstractTool::mousePress(const QPointF& scenePos)
{
    Q_UNUSED(scenePos)
}

void AbstractTool::mouseMove(const QPointF& scenePos)
{
    Q_UNUSED(scenePos)
}

void AbstractTool::mouseRelease(const QPointF& scenePos)
{
    Q_UNUSED(scenePos)
}

// ============================================================
// End Of File
// ============================================================
