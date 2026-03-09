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
// File        : src/core/Units.h
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#ifndef LogiKnots_UNITS_H
#define LogiKnots_UNITS_H

#include <QtGlobal>

namespace Units
{
    using mm_t = qint32;

    // Conversion utilitaire : mm -> qreal (pour QPainter/QGraphics)
    inline qreal mmToReal(mm_t mm) { return static_cast<qreal>(mm); }

    // Conversion utilitaire : qreal -> mm (arrondi)
    inline mm_t realToMM(qreal v) { return static_cast<mm_t>(qRound(v)); }
}

#endif

// ============================================================
// End Of File
// ============================================================
