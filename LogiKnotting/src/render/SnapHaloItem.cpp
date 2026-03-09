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
// File        : src/render/SnapHaloItem.cpp
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#include "SnapHaloItem.h"
#include <QPainter>

// Halo plus discret (diamètre 0.6 mm monde)
static constexpr qreal R = 0.5;

SnapHaloItem::SnapHaloItem()
{
    setZValue(20);
    setVisible(false);
}

void SnapHaloItem::setCenter(const QPointF& posMM)
{
    prepareGeometryChange();
    m_center = posMM;
    setPos(m_center);
    setVisible(true);
}

QRectF SnapHaloItem::boundingRect() const
{
    return QRectF(-R, -R, 4 * R, 4 * R);
}

void SnapHaloItem::paint(QPainter* p,
                         const QStyleOptionGraphicsItem*,
                         QWidget*)
{
    p->setRenderHint(QPainter::Antialiasing, true);

    QPen pen(QColor(10, 10, 255));  // bleu plus doux
    pen.setWidth(1);
    pen.setCosmetic(true);

    p->setPen(pen);
    p->setBrush(Qt::NoBrush);
    p->drawEllipse(QPointF(0, 0), R, R);
}

// ============================================================
// End Of File
// ============================================================
