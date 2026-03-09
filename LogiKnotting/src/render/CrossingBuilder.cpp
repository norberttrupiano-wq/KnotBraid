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
// File        : src/render/CrossingBuilder.cpp
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#include "CrossingBuilder.h"

#include <QLineF>      // ✅ OBLIGATOIRE
#include <QPointF>

namespace Render
{

static bool segmentIntersect(
    const Segment& s1,
    const Segment& s2,
    QPointF& out)
{
    QLineF l1(s1.a, s1.b);
    QLineF l2(s2.a, s2.b);

    QPointF p;
    if (l1.intersects(l2, &p) == QLineF::BoundedIntersection)
    {
        out = p;
        return true;
    }
    return false;
}

std::vector<Crossing> CrossingBuilder::build(const std::vector<Segment>& segments)
{
    std::vector<Crossing> crossings;

    const int n = static_cast<int>(segments.size());
    for (int i = 0; i < n; ++i)
    {
        for (int j = i + 2; j < n; ++j)
        {
            QPointF p;
            if (segmentIntersect(segments[i], segments[j], p))
            {
                Crossing c;
                c.position = p;
                c.over = true;
                crossings.push_back(c);
            }
        }
    }

    return crossings;
}

}

// ============================================================
// End Of File
// ============================================================
