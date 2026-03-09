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
// File        : src/render/SegmentItem.cpp
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#include "SegmentItem.h"
#include <QPainter>

// ------------------------------------------------------------
// Constructeur
// ------------------------------------------------------------
SegmentItem::SegmentItem(const QPointF& aMM, const QPointF& bMM)
    : m_line(aMM, bMM)
{
    setPos(0, 0);
}

// ------------------------------------------------------------
// BoundingRect (IMPORTANT si on élargit le trait)
// ------------------------------------------------------------
QRectF SegmentItem::boundingRect() const
{
    constexpr double margin = 6.0;   // doit être > demi-épaisseur
    return QRectF(m_line.p1(), m_line.p2())
        .normalized()
        .adjusted(-margin, -margin, margin, margin);
}

// ------------------------------------------------------------
// Paint
// ------------------------------------------------------------
void SegmentItem::paint(QPainter* p,
                        const QStyleOptionGraphicsItem*,
                        QWidget*)
{
    p->setRenderHint(QPainter::Antialiasing, true);

    QPen pen(QColor(0, 0, 0));

    pen.setWidthF(3.5);             // ← épaisseur confortable
    pen.setCosmetic(false);         // ← IMPORTANT : monde mm
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);

    p->setPen(pen);
    p->setBrush(Qt::NoBrush);

    p->drawLine(m_line);
}

// ============================================================
// End Of File
// ============================================================
