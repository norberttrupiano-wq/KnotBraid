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
// File        : src/core/SnapEngine.cpp
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#include "SnapEngine.h"

#include <cmath>

namespace Core
{

double SnapEngine::s_ribbonLength = 280.0;
double SnapEngine::s_ribbonOffset = 0.0;

// ------------------------------------------------------------
static double stepToMM(SnapStep step)
{
    switch (step)
    {
    case SnapStep::MM_1:  return 1.0;
    case SnapStep::MM_5:  return 5.0;
    case SnapStep::MM_10: return 10.0;
    }
    return 1.0;
}

// ------------------------------------------------------------
double SnapEngine::snapValue(double valueMM, double stepMM)
{
    // Arrondi métrique pur
    return std::round(valueMM / stepMM) * stepMM;
}

// ------------------------------------------------------------
// API publique (conservée)
// ------------------------------------------------------------
void SnapEngine::setRibbonLength(double lengthMM)
{
    s_ribbonLength = lengthMM;
}

void SnapEngine::setRibbonOffset(double offsetMM)
{
    s_ribbonOffset = offsetMM;
}

// ------------------------------------------------------------
// SNAP MÉTRIQUE PUR
// ------------------------------------------------------------
QPointF SnapEngine::snap(const QPointF& posMM, SnapStep step)
{
    const double s = stepToMM(step);
    return QPointF(
        snapValue(posMM.x(), s),
        snapValue(posMM.y(), s)
        );
}

}

// ============================================================
// End Of File
// ============================================================
