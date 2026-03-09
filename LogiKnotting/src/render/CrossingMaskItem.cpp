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
// File        : src/render/CrossingMaskItem.cpp
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#include "CrossingMaskItem.h"
#include <QPainter>

static constexpr qreal R = 1.5; // 3 px diamètre

CrossingMaskItem::CrossingMaskItem(const QPointF& centerMM)
    : m_center(centerMM)
{
    setPos(m_center);
    setZValue(10);                // au-dessus des segments
    setAcceptHoverEvents(true);   // prépare toolTip si besoin
}

QRectF CrossingMaskItem::boundingRect() const
{
    return QRectF(-R, -R, 2*R, 2*R);
}

void CrossingMaskItem::paint(QPainter* p,
                             const QStyleOptionGraphicsItem*,
                             QWidget*)
{
    p->setRenderHint(QPainter::Antialiasing, false);

    p->setPen(Qt::NoPen);
    p->setBrush(Qt::red);

    // cercle centré, net
    p->drawEllipse(QPointF(0, 0), R, R);
}

// ============================================================
// End Of File
// ============================================================
