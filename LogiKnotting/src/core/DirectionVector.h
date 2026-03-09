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
// File        : src/core/DirectionVector.h
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#pragma once

#include <QPointF>
#include <cmath>

class DirectionVector
{
public:
    DirectionVector(const QPointF& a, const QPointF& b)
    {
        m_dx = b.x() - a.x();
        m_dy = b.y() - a.y();
    }

    double dx() const { return m_dx; }
    double dy() const { return m_dy; }

    bool isZero() const
    {
        return (m_dx == 0.0 && m_dy == 0.0);
    }

    bool isCanonical() const
    {
        if (isZero())
            return false;

        if (m_dx == 0.0) return true;
        if (m_dy == 0.0) return true;
        if (std::abs(m_dx) == std::abs(m_dy)) return true;

        return false;
    }

    int angleDegrees() const
    {
        if (isZero())
            return -1;

        double angle = std::atan2(m_dy, m_dx) * 180.0 / M_PI;
        if (angle < 0.0)
            angle += 360.0;

        return static_cast<int>(std::round(angle));
    }

private:
    double m_dx = 0.0;
    double m_dy = 0.0;
};

// ============================================================
// End Of File
// ============================================================
