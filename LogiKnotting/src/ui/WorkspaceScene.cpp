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
// File        : src/ui/WorkspaceScene.cpp
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#include "WorkspaceScene.h"

#include "../render/SnapHaloItem.h"
#include "../model/WorkspaceModel.h"

#include <QPainter>
#include <QVector>
#include <QToolTip>
#include <QCursor>

#include <cmath>
#include <algorithm>
#include <array>

// -------------------------------------------------
// Helpers WRAP
// -------------------------------------------------
static inline double wrapDelta(double d, double L)
{
    // RamÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¨ne d dans [-L/2, +L/2] (delta minimal sur cylindre)
    if (L <= 0.0) return d;

    d = std::fmod(d, L);
    if (d >  L * 0.5) d -= L;
    if (d < -L * 0.5) d += L;
    return d;
}

// -------------------------------------------------
// Construction
// -------------------------------------------------
WorkspaceScene::WorkspaceScene(QObject* parent)
    : QGraphicsScene(parent)
{
    setSceneRect(-5000, -5000, 10000, 10000);

    m_snapHalo = new SnapHaloItem();
    m_snapHalo->setZValue(100000);
    m_snapHalo->setVisible(false);
    addItem(m_snapHalo);
}

void WorkspaceScene::setModel(Model::WorkspaceModel* model)
{
    m_model = model;

    // Reset cache (LK-STRICT)
    m_cacheSegmentsCount = 0;
    m_cacheCrossingsCount = 0;
    m_crossingsBySegB.clear();
    clearCrossingEditMarks();

    update();
}

void WorkspaceScene::setAnimationState(bool enabled, std::size_t completedSegments, double progress)
{
    m_animEnabled = enabled;
    m_animCompletedSegments = completedSegments;
    m_animProgress = progress;

    update();
}

void WorkspaceScene::setGridVisible(bool visible)
{
    if (m_gridVisible == visible)
        return;

    m_gridVisible = visible;
    update();
}

void WorkspaceScene::setStrokeWidthScale(double scale)
{
    const double clamped = std::clamp(scale, 0.5, 8.0);
    if (std::fabs(m_strokeWidthScale - clamped) < 1e-6)
        return;

    m_strokeWidthScale = clamped;
    update();
}

void WorkspaceScene::setPrintStrokeWidthMM(double widthMM)
{
    const double clamped = std::clamp(widthMM, 0.0, 10.0);
    if (std::fabs(m_printStrokeWidthMM - clamped) < 1e-6)
        return;

    m_printStrokeWidthMM = clamped;
    update();
}

// -------------------------------------------------
// Halo SNAP (affichÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â© / dÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©roulÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©)
// -------------------------------------------------
void WorkspaceScene::updateSnapHalo(const QPointF& posMM)
{
    if (m_hasLastSnap && posMM == m_lastSnapPosMM)
        return;

    m_lastSnapPosMM = posMM;
    m_hasLastSnap   = true;

    if (m_snapHalo)
    {
        m_snapHalo->setVisible(true);
        m_snapHalo->setCenter(posMM);
    }
}

void WorkspaceScene::hideSnapHalo()
{
    if (m_snapHalo)
        m_snapHalo->setVisible(false);
    m_hasLastSnap = false;
}

// -------------------------------------------------
// Ajout point (vÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©ritÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â© modÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¨le)
// -------------------------------------------------
void WorkspaceScene::addPoint(const QPointF& posMM)
{
    if (!m_model)
        return;

    // IMPORTANT : API canonique
    m_model->addPointMM(posMM);

    // Cache potentiellement invalide (nouveaux segments/croisements)
    m_cacheSegmentsCount = 0;
    m_cacheCrossingsCount = 0;

    update();
}

// -------------------------------------------------
// Grille (mm rÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©els ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â monde mÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©trique pur)
// -------------------------------------------------
void WorkspaceScene::drawGrid(QPainter* p, const QRectF& rect)
{
    const int step1  = 1;
    const int step5  = 5;
    const int step10 = 10;

    QPen pen1(QColor(0, 0, 0));
    QPen pen5(QColor(40, 50, 255));
    QPen pen10(QColor(255, 50, 40));

    pen1.setCosmetic(true);
    pen5.setCosmetic(true);
    pen5.setWidthF(1.1);
    pen10.setCosmetic(true);
    pen10.setWidthF(1.5);

    const int left   = static_cast<int>(std::floor(rect.left()));
    const int right  = static_cast<int>(std::ceil (rect.right()));
    const int top    = static_cast<int>(std::floor(rect.top()));
    const int bottom = static_cast<int>(std::ceil (rect.bottom()));

    // Verticales
    for (int x = left; x <= right; ++x)
    {
        if (x % step10 == 0)      p->setPen(pen10);
        else if (x % step5 == 0)  p->setPen(pen5);
        else                      p->setPen(pen1);

        p->drawLine(x, top, x, bottom);
    }

    // Horizontales
    for (int y = top; y <= bottom; ++y)
    {
        if (y % step10 == 0)      p->setPen(pen10);
        else if (y % step5 == 0)  p->setPen(pen5);
        else                      p->setPen(pen1);

        p->drawLine(left, y, right, y);
    }
}

// -------------------------------------------------
// Cache perf : crossings par segmentBIndex
// -------------------------------------------------
void WorkspaceScene::rebuildCrossingsCacheIfNeeded(std::size_t totalSegments)
{
    if (!m_model)
        return;

    const auto& crossings = m_model->crossings();
    const std::size_t cCount = crossings.size();

    if (m_cacheSegmentsCount == totalSegments && m_cacheCrossingsCount == cCount && !m_crossingsBySegB.empty())
        return;

    m_cacheSegmentsCount  = totalSegments;
    m_cacheCrossingsCount = cCount;

    m_crossingsBySegB.clear();
    m_crossingsBySegB.resize(totalSegments);

    for (const auto& c : crossings)
    {
        if (c.segmentBIndex < 0)
            continue;

        const std::size_t segB = static_cast<std::size_t>(c.segmentBIndex);
        if (segB >= totalSegments)
            continue;

        m_crossingsBySegB[segB].push_back(&c);
    }
}

// -------------------------------------------------
// Points + Segments (WRAP ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â°TAPE 1)
// - ModÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¨le : points LOGIQUES (0..L-1)
// - Rendu  : polyline ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦ÃƒÂ¢Ã¢â€šÂ¬Ã…â€œliftÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©eÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â + copies ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â±k*L qui couvrent rect
//
// PERF :
// - Indexation crossingsBySegB ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©vite le scan complet par segment ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â  chaque frame.
// -------------------------------------------------
void WorkspaceScene::drawPointsAndSegmentsFromTopology(QPainter* p, const QRectF& rect)
{
    if (!m_model)
        return;

    const auto& topo = m_model->topologySnapshot();
    const double L = static_cast<double>(topo.ribbonLengthMM);
    if (L <= 0.0)
        return;

    const auto& segments = topo.segments;
    const std::size_t totalSegments = segments.size();

    const std::size_t completedSegments =
        m_animEnabled ? std::min<std::size_t>(m_animCompletedSegments, totalSegments)
                      : totalSegments;

    const bool hasPartial =
        m_animEnabled &&
        (completedSegments < totalSegments) &&
        (m_animProgress > 0.0) &&
        (m_animProgress < 1.0);

    std::array<int, Domain::MaxRopes> maxVisiblePoint;
    maxVisiblePoint.fill(-1);

    std::map<Domain::SegmentRef, std::size_t> segmentOrder;
    std::map<Domain::SegmentRef, const Domain::TopoSegment*> segmentByRef;
    segmentOrder.clear();
    segmentByRef.clear();

    for (std::size_t i = 0; i < segments.size(); ++i)
    {
        segmentOrder[segments[i].ref] = i;
        segmentByRef[segments[i].ref] = &segments[i];
    }

    auto isEndpointIntersection = [](const QLineF& l1, const QLineF& l2, const QPointF& inter) -> bool
    {
        constexpr double eps = 1e-6;
        return (QLineF(inter, l1.p1()).length() <= eps) ||
               (QLineF(inter, l1.p2()).length() <= eps) ||
               (QLineF(inter, l2.p1()).length() <= eps) ||
               (QLineF(inter, l2.p2()).length() <= eps);
    };

    auto preciseCrossingPosForShift =
        [&](const Domain::TopoCrossing& c,
            const Domain::SegmentRef& anchorRef,
            int anchorShiftK,
            QPointF* outPos) -> bool
    {
        if (L <= 0.0 || !outPos)
            return false;

        const auto itAnchor = segmentByRef.find(anchorRef);
        if (itAnchor == segmentByRef.end() || !itAnchor->second)
            return false;

        const Domain::SegmentRef otherRef = (anchorRef == c.s1) ? c.s2 : c.s1;
        const auto itOther = segmentByRef.find(otherRef);
        if (itOther == segmentByRef.end() || !itOther->second)
            return false;

        const Domain::TopoSegment& anchorSeg = *itAnchor->second;
        const Domain::TopoSegment& otherSeg = *itOther->second;

        const double anchorShift = static_cast<double>(anchorShiftK) * L;
        const QLineF la(
            QPointF(static_cast<double>(anchorSeg.a.xAbs) + anchorShift, static_cast<double>(anchorSeg.a.y)),
            QPointF(static_cast<double>(anchorSeg.b.xAbs) + anchorShift, static_cast<double>(anchorSeg.b.y))
        );

        const double aMin = std::min(la.p1().x(), la.p2().x());
        const double aMax = std::max(la.p1().x(), la.p2().x());
        const double bMin0 = static_cast<double>(std::min(otherSeg.a.xAbs, otherSeg.b.xAbs));
        const double bMax0 = static_cast<double>(std::max(otherSeg.a.xAbs, otherSeg.b.xAbs));

        const int kMin = static_cast<int>(std::floor((aMin - bMax0) / L)) - 1;
        const int kMax = static_cast<int>(std::ceil((aMax - bMin0) / L)) + 1;

        const double targetX = static_cast<double>(c.xAbs) + anchorShift;
        bool has = false;
        QPointF best;
        double bestScore = 1e100;

        for (int kOther = kMin; kOther <= kMax; ++kOther)
        {
            const double otherShift = static_cast<double>(kOther) * L;
            const QLineF lb(
                QPointF(static_cast<double>(otherSeg.a.xAbs) + otherShift, static_cast<double>(otherSeg.a.y)),
                QPointF(static_cast<double>(otherSeg.b.xAbs) + otherShift, static_cast<double>(otherSeg.b.y))
            );

            QPointF inter;
            if (la.intersects(lb, &inter) != QLineF::BoundedIntersection)
                continue;

            if (isEndpointIntersection(la, lb, inter))
                continue;

            const double score = std::abs(inter.x() - targetX);
            if (!has || score < bestScore)
            {
                bestScore = score;
                best = inter;
                has = true;
            }
        }

        if (!has)
            return false;

        *outPos = best;
        return true;
    };

    auto segmentVisibility = [&](const Domain::SegmentRef& ref) -> int
    {
        const auto it = segmentOrder.find(ref);
        if (it == segmentOrder.end())
            return 0;

        const std::size_t idx = it->second;
        if (idx < completedSegments)
            return 2; // full

        if (hasPartial && idx == completedSegments)
            return 1; // partial

        return 0;
    };

    std::map<Domain::SegmentRef, std::vector<const Domain::TopoCrossing*>> underBySegment;
    std::vector<const Domain::TopoCrossing*> visibleCrossings;
    underBySegment.clear();
    visibleCrossings.clear();

    for (const auto& c0 : topo.crossings)
    {
        const int vis1 = segmentVisibility(c0.s1);
        const int vis2 = segmentVisibility(c0.s2);

        if (vis1 == 0 || vis2 == 0)
            continue;

        // In animation: avoid cutting/overlaying crossings with the currently partial segment.
        if (vis1 == 1 || vis2 == 1)
            continue;

        visibleCrossings.push_back(&c0);

        const bool s2OverS1 = c0.s2OverS1;
        const Domain::SegmentRef underRef = s2OverS1 ? c0.s1 : c0.s2;
        underBySegment[underRef].push_back(&c0);
    }

    const qreal strokeScale = static_cast<qreal>(std::clamp(m_strokeWidthScale, 0.5, 8.0));
    const bool usePhysicalStrokeWidth = (m_printStrokeWidthMM > 0.05);
    const qreal lightStrokeWidthMM = static_cast<qreal>(m_printStrokeWidthMM);
    const qreal darkStrokeWidthMM = lightStrokeWidthMM * (7.0 / 13.0);
    const auto scaledWidth = [strokeScale](qreal baseWidth)
    {
        return std::max<qreal>(0.5, baseWidth * strokeScale);
    };
    const auto configureStrokePen = [&](QPen& pen, qreal baseWidthPx, qreal widthMM)
    {
        pen.setCosmetic(!usePhysicalStrokeWidth);
        pen.setWidthF(usePhysicalStrokeWidth ? std::max<qreal>(0.10, widthMM)
                                             : scaledWidth(baseWidthPx));
    };

    const double holeHalfLenMM = usePhysicalStrokeWidth
        ? std::max(0.70, static_cast<double>(lightStrokeWidthMM) * 0.38)
        : 0.70 * static_cast<double>(strokeScale);
    const double crossingOverlapMM = usePhysicalStrokeWidth
        ? std::max(0.05, static_cast<double>(lightStrokeWidthMM) * 0.03)
        : 0.05 * static_cast<double>(strokeScale);
    const double overHalfLenMM = holeHalfLenMM + crossingOverlapMM;

    const bool aaPrev = p->renderHints().testFlag(QPainter::Antialiasing);
    p->setRenderHint(QPainter::Antialiasing, true);

    struct Interval { double t0 = 0.0; double t1 = 0.0; };

    auto drawSegmentPass =
        [&](const Domain::SegmentRef& ref,
            const QPointF& a,
            const QPointF& b,
            const QColor& base,
            const std::vector<Interval>* holes)
    {
        QPen segPenLightRound(base.lighter(150));
        configureStrokePen(segPenLightRound, 13.0, lightStrokeWidthMM);
        segPenLightRound.setCapStyle(Qt::RoundCap);
        segPenLightRound.setJoinStyle(Qt::RoundJoin);

        QPen segPenDarkRound(base.darker(145));
        configureStrokePen(segPenDarkRound, 7.0, darkStrokeWidthMM);
        segPenDarkRound.setCapStyle(Qt::RoundCap);
        segPenDarkRound.setJoinStyle(Qt::RoundJoin);

        QPen segPenLightFlat = segPenLightRound;
        segPenLightFlat.setCapStyle(Qt::FlatCap);
        QPen segPenDarkFlat = segPenDarkRound;
        segPenDarkFlat.setCapStyle(Qt::FlatCap);

        auto drawSub = [&](double t0, double t1)
        {
            if (t1 <= t0)
                return;

            const QLineF line(a, b);
            const QPointF p0 = line.pointAt(t0);
            const QPointF p1 = line.pointAt(t1);

            const bool hasCuts = (holes && !holes->empty());
            p->setPen(hasCuts ? segPenLightFlat : segPenLightRound);
            p->drawLine(p0, p1);

            p->setPen(hasCuts ? segPenDarkFlat : segPenDarkRound);
            p->drawLine(p0, p1);
        };

        Q_UNUSED(ref)

        if (!holes || holes->empty())
        {
            drawSub(0.0, 1.0);
            return;
        }

        double curT = 0.0;
        for (const Interval& h : *holes)
        {
            drawSub(curT, h.t0);
            curT = h.t1;
        }
        drawSub(curT, 1.0);
    };

    auto drawWrappedSegment =
        [&](const Domain::TopoSegment& seg,
            double part01,
            const std::vector<const Domain::TopoCrossing*>* underCrossings)
    {
        const double x0 = static_cast<double>(seg.a.xAbs);
        const double y0 = static_cast<double>(seg.a.y);
        const double x1Full = static_cast<double>(seg.b.xAbs);
        const double y1Full = static_cast<double>(seg.b.y);

        const double p01 = std::clamp(part01, 0.0, 1.0);
        const double x1 = x0 + p01 * (x1Full - x0);
        const double y1 = y0 + p01 * (y1Full - y0);

        const QColor base = ropeColor(seg.ref.ropeId);

        const double xMin = std::min(x0, x1);
        const double xMax = std::max(x0, x1);
        const int kMin = static_cast<int>(std::floor((rect.left() - xMax) / L)) - 1;
        const int kMax = static_cast<int>(std::ceil((rect.right() - xMin) / L)) + 1;

        for (int k = kMin; k <= kMax; ++k)
        {
            const double shift = static_cast<double>(k) * L;
            const QPointF a(x0 + shift, y0);
            const QPointF b(x1 + shift, y1);

            std::vector<Interval> holes;

            // No holes for partial segments to keep animation stable and pedagogic.
            if (underCrossings && p01 >= 0.999)
            {
                const QLineF line(a, b);
                const double len = line.length();
                if (len > 1e-9)
                {
                    const double dt = holeHalfLenMM / len;
                    holes.reserve(underCrossings->size());

                    for (const Domain::TopoCrossing* pc : *underCrossings)
                    {
                        QPointF crossPos(static_cast<double>(pc->xAbs) + shift,
                                         static_cast<double>(pc->y));
                        (void)preciseCrossingPosForShift(*pc, seg.ref, k, &crossPos);

                        double t = 0.0;
                        if (std::fabs(b.x() - a.x()) >= std::fabs(b.y() - a.y()))
                        {
                            const double denom = (b.x() - a.x());
                            if (std::fabs(denom) > 1e-9)
                                t = (crossPos.x() - a.x()) / denom;
                        }
                        else
                        {
                            const double denom = (b.y() - a.y());
                            if (std::fabs(denom) > 1e-9)
                                t = (crossPos.y() - a.y()) / denom;
                        }

                        t = std::clamp(t, 0.0, 1.0);

                        Interval in;
                        in.t0 = std::clamp(t - dt, 0.0, 1.0);
                        in.t1 = std::clamp(t + dt, 0.0, 1.0);

                        if (in.t1 > in.t0)
                            holes.push_back(in);
                    }

                    if (!holes.empty())
                    {
                        std::sort(holes.begin(), holes.end(),
                                  [](const Interval& a0, const Interval& b0)
                                  {
                                      return a0.t0 < b0.t0;
                                  });

                        std::vector<Interval> merged;
                        merged.reserve(holes.size());

                        Interval cur = holes[0];
                        for (std::size_t i = 1; i < holes.size(); ++i)
                        {
                            if (holes[i].t0 <= cur.t1)
                                cur.t1 = std::max(cur.t1, holes[i].t1);
                            else
                            {
                                merged.push_back(cur);
                                cur = holes[i];
                            }
                        }
                        merged.push_back(cur);
                        holes.swap(merged);
                    }
                }
            }

            drawSegmentPass(seg.ref, a, b, base, holes.empty() ? nullptr : &holes);
        }
    };

    for (std::size_t gi = 0; gi < completedSegments; ++gi)
    {
        const auto& seg = segments[gi];
        const auto itUnder = underBySegment.find(seg.ref);
        const std::vector<const Domain::TopoCrossing*>* underList =
            (itUnder != underBySegment.end()) ? &itUnder->second : nullptr;

        drawWrappedSegment(seg, 1.0, underList);

        const std::size_t rid = static_cast<std::size_t>(seg.ref.ropeId);
        if (rid < maxVisiblePoint.size())
            maxVisiblePoint[rid] = std::max(maxVisiblePoint[rid], seg.ref.segIndex + 1);
    }

    if (hasPartial)
    {
        const auto& seg = segments[completedSegments];
        drawWrappedSegment(seg, m_animProgress, nullptr);

        const std::size_t rid = static_cast<std::size_t>(seg.ref.ropeId);
        if (rid < maxVisiblePoint.size())
            maxVisiblePoint[rid] = std::max(maxVisiblePoint[rid], seg.ref.segIndex);
    }

    // Draw over-pass at crossing center (thumbnail rendering rule).
    for (const Domain::TopoCrossing* pc : visibleCrossings)
    {
        if (!pc)
            continue;

        const bool s2OverS1 = pc->s2OverS1;
        const Domain::SegmentRef overRef = s2OverS1 ? pc->s2 : pc->s1;

        const auto itSeg = segmentByRef.find(overRef);
        if (itSeg == segmentByRef.end() || !itSeg->second)
            continue;

        const Domain::TopoSegment& seg = *itSeg->second;

        const double x0 = static_cast<double>(seg.a.xAbs);
        const double y0 = static_cast<double>(seg.a.y);
        const double x1 = static_cast<double>(seg.b.xAbs);
        const double y1 = static_cast<double>(seg.b.y);

        const double vx = x1 - x0;
        const double vy = y1 - y0;
        const double vLen = std::sqrt(vx * vx + vy * vy);
        if (vLen <= 1e-9)
            continue;

        const double ux = vx / vLen;
        const double uy = vy / vLen;

        const QColor base = ropeColor(overRef.ropeId);

        QPen segPenLightRound(base.lighter(150));
        configureStrokePen(segPenLightRound, 13.0, lightStrokeWidthMM);
        segPenLightRound.setCapStyle(Qt::RoundCap);
        segPenLightRound.setJoinStyle(Qt::RoundJoin);

        QPen segPenDarkRound(base.darker(145));
        configureStrokePen(segPenDarkRound, 7.0, darkStrokeWidthMM);
        segPenDarkRound.setCapStyle(Qt::RoundCap);
        segPenDarkRound.setJoinStyle(Qt::RoundJoin);

        QPen segPenLightFlat = segPenLightRound;
        segPenLightFlat.setCapStyle(Qt::FlatCap);
        QPen segPenDarkFlat = segPenDarkRound;
        segPenDarkFlat.setCapStyle(Qt::FlatCap);

        const double xCross = static_cast<double>(pc->xAbs);
        const double yCross = static_cast<double>(pc->y);

        const int kMin = static_cast<int>(std::floor((rect.left() - xCross) / L)) - 1;
        const int kMax = static_cast<int>(std::ceil((rect.right() - xCross) / L)) + 1;

        for (int k = kMin; k <= kMax; ++k)
        {
            QPointF cPos(xCross + static_cast<double>(k) * L, yCross);
            (void)preciseCrossingPosForShift(*pc, overRef, k, &cPos);
            const QPointF a(cPos.x() - ux * overHalfLenMM,
                            cPos.y() - uy * overHalfLenMM);
            const QPointF b(cPos.x() + ux * overHalfLenMM,
                            cPos.y() + uy * overHalfLenMM);

            p->setPen(segPenLightFlat);
            p->drawLine(a, b);

            p->setPen(segPenDarkFlat);
            p->drawLine(a, b);
        }
    }


    if (!m_animEnabled)
    {
        for (const auto& rope : topo.ropes)
        {
            const std::size_t rid = static_cast<std::size_t>(rope.ropeId);
            if (rid < maxVisiblePoint.size())
                maxVisiblePoint[rid] = static_cast<int>(rope.points.size()) - 1;
        }
    }

    for (const auto& rope : topo.ropes)
    {
        if (rope.points.empty())
            continue;

        const QColor base = ropeColor(rope.ropeId);

        QPen pointPen(base.lighter(170));
        configureStrokePen(pointPen, 1.5, lightStrokeWidthMM * (1.5 / 13.0));

        const QBrush pointBrush(base);
        const double pointR = 0.35;

        p->setPen(pointPen);
        p->setBrush(pointBrush);

        const std::size_t rid = static_cast<std::size_t>(rope.ropeId);
        int maxPt = -1;
        if (rid < maxVisiblePoint.size())
            maxPt = maxVisiblePoint[rid];

        if (maxPt < 0)
            continue;

        const int last = std::min<int>(maxPt, static_cast<int>(rope.points.size()) - 1);

        for (int pi = 0; pi <= last; ++pi)
        {
            const auto& pt = rope.points[static_cast<std::size_t>(pi)];
            const double x = static_cast<double>(pt.xAbs);
            const double y = static_cast<double>(pt.y);

            const int kMin = static_cast<int>(std::floor((rect.left() - x) / L)) - 1;
            const int kMax = static_cast<int>(std::ceil((rect.right() - x) / L)) + 1;

            for (int k = kMin; k <= kMax; ++k)
            {
                const double shift = static_cast<double>(k) * L;
                p->drawEllipse(QPointF(x + shift, y), pointR, pointR);
            }
        }
    }

    p->setRenderHint(QPainter::Antialiasing, aaPrev);
}
void WorkspaceScene::drawPointsAndSegments(QPainter* p, const QRectF& rect)
{
    if (!m_model)
        return;

    // MIGRATION-PARALLEL: rendu multi-cordes depuis TopologyStore.
    // Le chemin legacy reste prÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©sent ci-dessous comme fallback.
    {
        const auto& topo = m_model->topologySnapshot();
        bool hasTopoPoints = false;
        for (const auto& rope : topo.ropes)
        {
            if (!rope.points.empty())
            {
                hasTopoPoints = true;
                break;
            }
        }

        if (hasTopoPoints)
        {
            drawPointsAndSegmentsFromTopology(p, rect);
            return;
        }
    }

    const auto& pts = m_model->points();
    if (pts.empty())
        return;

    const double L = static_cast<double>(m_model->ribbonLengthMM());
    if (L <= 0.0)
        return;

    const std::size_t totalSegments = (pts.size() >= 2) ? (pts.size() - 1) : 0;

    // PERF cache rebuild (safe)
    rebuildCrossingsCacheIfNeeded(totalSegments);

    const std::size_t completedSegments =
        m_animEnabled ? std::min<std::size_t>(m_animCompletedSegments, totalSegments)
                      : totalSegments;

    const bool hasPartial =
        m_animEnabled &&
        (completedSegments < totalSegments) &&
        (m_animProgress > 0.0) &&
        (m_animProgress < 1.0);

    const std::size_t pointCount = (totalSegments == 0) ? 1 : (completedSegments + 1);

    // ---------------------------------------------------------
    // 1) Polyline liftÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©e (dÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©roulage cylindre) ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â limitÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©e (animation)
    // ---------------------------------------------------------
    QVector<QPointF> base;
    base.reserve(static_cast<int>(pointCount + 1));

    const double x0Log   = pts[0].x();
    const double targetX = rect.center().x();
    const double k0      = std::llround((targetX - x0Log) / L);

    double xPrevLift = x0Log + k0 * L;
    double xPrevLog  = x0Log;

    base.append(QPointF(xPrevLift, pts[0].y()));

    for (std::size_t i = 1; i < pointCount; ++i)
    {
        const double xLog = pts[i].x();
        double d = xLog - xPrevLog;
        d = wrapDelta(d, L);

        xPrevLift += d;
        xPrevLog   = xLog;

        base.append(QPointF(xPrevLift, pts[i].y()));
    }

    // Calcul du point partiel (mode Serpent) en repÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¨re liftÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©
    QPointF partialLiftedEnd;
    bool    usePartial = false;

    if (hasPartial)
    {
        const std::size_t i0 = completedSegments;
        const std::size_t i1 = completedSegments + 1;

        const double x1Log = pts[i1].x();
        double d = x1Log - xPrevLog;
        d = wrapDelta(d, L);

        const double y0 = pts[i0].y();
        const double y1 = pts[i1].y();

        const double p01 = std::clamp(m_animProgress, 0.0, 1.0);

        partialLiftedEnd = QPointF(xPrevLift + p01 * d,
                                   y0 + p01 * (y1 - y0));
        usePartial = true;
    }

    double minX = base[0].x();
    double maxX = base[0].x();
    for (const QPointF& q : base)
    {
        minX = std::min(minX, q.x());
        maxX = std::max(maxX, q.x());
    }

    if (usePartial)
    {
        minX = std::min(minX, partialLiftedEnd.x());
        maxX = std::max(maxX, partialLiftedEnd.x());
    }

    const double left  = rect.left();
    const double right = rect.right();

    const int kMin = static_cast<int>(std::floor((left  - maxX) / L)) - 1;
    const int kMax = static_cast<int>(std::ceil ((right - minX) / L)) + 1;

    // ---------------------------------------------------------
    // STYLE RUBAN 2ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“4ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“2 (15 / 9 px)
    // ---------------------------------------------------------
    const qreal strokeScale = static_cast<qreal>(std::clamp(m_strokeWidthScale, 0.5, 8.0));
    const bool usePhysicalStrokeWidth = (m_printStrokeWidthMM > 0.05);
    const qreal lightStrokeWidthMM = static_cast<qreal>(m_printStrokeWidthMM);
    const qreal darkStrokeWidthMM = lightStrokeWidthMM * (9.0 / 15.0);
    const auto scaledWidth = [strokeScale](qreal baseWidth)
    {
        return std::max<qreal>(0.5, baseWidth * strokeScale);
    };
    const auto configureStrokePen = [&](QPen& pen, qreal baseWidthPx, qreal widthMM)
    {
        pen.setCosmetic(!usePhysicalStrokeWidth);
        pen.setWidthF(usePhysicalStrokeWidth ? std::max<qreal>(0.10, widthMM)
                                             : scaledWidth(baseWidthPx));
    };

    QPen segPenLight(QColor(120, 170, 255, 255));
    configureStrokePen(segPenLight, 15.0, lightStrokeWidthMM);
    segPenLight.setCapStyle(Qt::RoundCap);
    segPenLight.setJoinStyle(Qt::RoundJoin);

    QPen segPenDark(QColor(0, 70, 200, 255));
    configureStrokePen(segPenDark, 9.0, darkStrokeWidthMM);
    segPenDark.setCapStyle(Qt::RoundCap);
    segPenDark.setJoinStyle(Qt::RoundJoin);

    // Points intÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©grÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©s au ruban
    QColor edgeColor(160, 200, 255, 255);
    QColor coreColor(0, 60, 255, 255);

    QPen pointPen(edgeColor);
    configureStrokePen(pointPen, 2.0, lightStrokeWidthMM * (2.0 / 15.0));

    const double r = 0.01; // quasi invisible (intÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©grÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â© au segment)

    // Taille du "trou" (mm en coordonnÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©es scÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¨ne)
    const double holeHalfLenMM = usePhysicalStrokeWidth
        ? std::max(1.1, static_cast<double>(lightStrokeWidthMM) * 0.55)
        : 1.1 * static_cast<double>(strokeScale);

    struct Interval { double t0; double t1; };

    auto drawSegmentWithHoles = [&](const QPointF& a,
                                    const QPointF& b,
                                    int segIndex,
                                    double shift,
                                    double segStartXLog,
                                    double segStartXLift)
    {
        const QLineF seg(a, b);
        const double len = seg.length();
        if (len <= 1e-9)
            return;

        std::vector<Interval> holes;
        holes.reserve(8);

        const double dt = holeHalfLenMM / len;

        // PERF : ne regarde QUE les crossings du segmentBIndex courant
        if (segIndex >= 0)
        {
            const std::size_t s = static_cast<std::size_t>(segIndex);
            if (s < m_crossingsBySegB.size())
            {
                for (const Model::Crossing* pc : m_crossingsBySegB[s])
                {
                    const Model::Crossing& c = *pc;

                    if (c.newSegmentOver)
                        continue;

                    // FIX WRAP : crossing dans repÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¨re liftÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â© du segment
                    const double xCrossLog = c.positionMM.x();
                    double dx = xCrossLog - segStartXLog;
                    dx = wrapDelta(dx, L);
                    const double xCrossLift = segStartXLift + dx;

                    const QPointF crossPos(xCrossLift + shift, c.positionMM.y());

                    double t = 0.0;
                    if (std::fabs(b.x() - a.x()) >= std::fabs(b.y() - a.y()))
                    {
                        const double denom = (b.x() - a.x());
                        if (std::fabs(denom) > 1e-9)
                            t = (crossPos.x() - a.x()) / denom;
                    }
                    else
                    {
                        const double denom = (b.y() - a.y());
                        if (std::fabs(denom) > 1e-9)
                            t = (crossPos.y() - a.y()) / denom;
                    }

                    t = std::clamp(t, 0.0, 1.0);

                    Interval in;
                    in.t0 = std::clamp(t - dt, 0.0, 1.0);
                    in.t1 = std::clamp(t + dt, 0.0, 1.0);

                    if (in.t1 > in.t0)
                        holes.push_back(in);
                }
            }
        }

        if (!holes.empty())
        {
            std::sort(holes.begin(), holes.end(),
                      [](const Interval& a0, const Interval& b0){ return a0.t0 < b0.t0; });

            std::vector<Interval> merged;
            merged.reserve(holes.size());

            Interval cur = holes[0];
            for (std::size_t i = 1; i < holes.size(); ++i)
            {
                if (holes[i].t0 <= cur.t1)
                    cur.t1 = std::max(cur.t1, holes[i].t1);
                else
                {
                    merged.push_back(cur);
                    cur = holes[i];
                }
            }
            merged.push_back(cur);
            holes.swap(merged);
        }

        auto drawSub = [&](double t0, double t1)
        {
            if (t1 <= t0)
                return;

            QPointF p0 = seg.pointAt(t0);
            QPointF p1 = seg.pointAt(t1);

            p->setPen(segPenLight);
            p->drawLine(p0, p1);

            p->setPen(segPenDark);
            p->drawLine(p0, p1);
        };


        if (holes.empty())
        {
            drawSub(0.0, 1.0);
            return;
        }

        double curT = 0.0;
        for (const Interval& h : holes)
        {
            drawSub(curT, h.t0);
            curT = h.t1;
        }
        drawSub(curT, 1.0);
    };

    const bool aaPrev = p->renderHints().testFlag(QPainter::Antialiasing);
    p->setRenderHint(QPainter::Antialiasing, true);

    for (int k = kMin; k <= kMax; ++k)
    {
        const double shift = static_cast<double>(k) * L;

        // Segments complets [0..completedSegments-1]
        for (std::size_t i = 1; i < base.size(); ++i)
        {
            const QPointF a(base[static_cast<int>(i - 1)].x() + shift,
                            base[static_cast<int>(i - 1)].y());
            const QPointF b(base[static_cast<int>(i)].x() + shift,
                            base[static_cast<int>(i)].y());

            const int segIndex = static_cast<int>(i - 1);

            const double segStartXLog  = pts[i - 1].x();
            const double segStartXLift = base[static_cast<int>(i - 1)].x();

            drawSegmentWithHoles(a, b, segIndex, shift, segStartXLog, segStartXLift);
        }

        // Segment partiel (serpent) : sans trous (pÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©dagogique)
        if (usePartial)
        {
            const QPointF a(base.back().x() + shift, base.back().y());
            const QPointF b(partialLiftedEnd.x() + shift, partialLiftedEnd.y());

            p->setPen(segPenLight);
            p->drawLine(a, b);

            p->setPen(segPenDark);
            p->drawLine(a, b);
        }


        // Points (quasi invisibles)
        p->setPen(pointPen);
        p->setBrush(QBrush(coreColor));
        for (const QPointF& q : base)
        {
            const QPointF qq(q.x() + shift, q.y());
            p->drawEllipse(qq, r, r);
        }
    }

    p->setRenderHint(QPainter::Antialiasing, aaPrev);
}

// -------------------------------------------------
// Tooltip croisement survolÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©
// -------------------------------------------------
void WorkspaceScene::refreshHoveredCrossing()
{
    if (!m_model)
        return;

    const RenderCrossing rc = renderCrossingAt(m_hoveredCrossingIndex);
    if (!rc.valid)
        return;

    const QPoint tipPos = QCursor::pos() + QPoint(0, -24);
    QToolTip::showText(tipPos, crossingTooltipText(rc, m_hoveredCrossingIndex, true), nullptr, QRect(), 10000);
}

void WorkspaceScene::refreshActiveCrossingTooltip()
{
    if (!m_model)
        return;

    const RenderCrossing rc = renderCrossingAt(m_activeCrossingIndex);
    if (!rc.valid)
        return;

    QToolTip::showText(QCursor::pos(), crossingTooltipText(rc, m_activeCrossingIndex, true));
}

void WorkspaceScene::refreshActiveCrossingTooltipAt(const QPoint& globalPos)
{
    if (!m_model)
        return;

    const RenderCrossing rc = renderCrossingAt(m_activeCrossingIndex);
    if (!rc.valid)
        return;

    QToolTip::showText(globalPos, crossingTooltipText(rc, m_activeCrossingIndex, true));
}

bool WorkspaceScene::useTopologyCrossings() const
{
    if (!m_model)
        return false;

    const auto& topo = m_model->topologySnapshot();
    return !topo.crossings.empty();
}

WorkspaceScene::RenderCrossing WorkspaceScene::renderCrossingAt(int idx) const
{
    RenderCrossing rc;
    if (!m_model || idx < 0)
        return rc;

    if (useTopologyCrossings())
    {
        const auto& topo = m_model->topologySnapshot();
        if (idx >= static_cast<int>(topo.crossings.size()))
            return rc;

        const auto& c = topo.crossings[static_cast<std::size_t>(idx)];
        rc.valid = true;
        rc.topologyBased = true;
        rc.index = idx;
        rc.absPosMM = QPointF(static_cast<double>(c.xAbs), static_cast<double>(c.y));
        rc.tour = c.turn;
        rc.s1 = c.s1;
        rc.s2 = c.s2;
        rc.s2OverS1 = c.s2OverS1;
        rc.key = c.key;
        return rc;
    }

    const auto& crossings = m_model->crossings();
    if (idx >= static_cast<int>(crossings.size()))
        return rc;

    const auto& c = crossings[static_cast<std::size_t>(idx)];
    const int L = m_model->ribbonLengthMM();
    if (L <= 0)
        return rc;

    rc.valid = true;
    rc.topologyBased = false;
    rc.index = idx;
    rc.tour = c.tour;
    rc.absPosMM = QPointF(c.positionMM.x() + static_cast<double>(c.tour) * static_cast<double>(L),
                          c.positionMM.y());
    rc.legacySegmentA = c.segmentAIndex;
    rc.legacySegmentB = c.segmentBIndex;
    rc.legacyNewSegmentOver = c.newSegmentOver;

    return rc;
}

QColor WorkspaceScene::ropeColor(Domain::RopeId ropeId) const
{
    static const QColor kFallback[Domain::MaxRopes] = {
        QColor(40, 120, 255),
        QColor(230, 70, 70),
        QColor(60, 170, 95),
        QColor(240, 170, 40),
        QColor(150, 90, 220)
    };

    if (!m_model)
        return kFallback[static_cast<std::size_t>(ropeId) % static_cast<std::size_t>(Domain::MaxRopes)];

    const auto& topo = m_model->topologySnapshot();
    const std::size_t idx = static_cast<std::size_t>(ropeId);
    if (idx < topo.ropes.size() && topo.ropes[idx].color.isValid())
        return topo.ropes[idx].color;

    return kFallback[idx % static_cast<std::size_t>(Domain::MaxRopes)];
}

bool WorkspaceScene::crossingOverState(const RenderCrossing& rc) const
{
    if (!rc.valid)
        return true;

    if (!rc.topologyBased)
        return rc.legacyNewSegmentOver;

    return rc.s2OverS1;
}

QString WorkspaceScene::crossingTooltipText(const RenderCrossing& rc,
                                            int /*crossingIndex*/,
                                            bool /*includeEditHints*/) const
{
    if (!m_model || !rc.valid)
        return QString();

    auto tooltipFromLine = [this](const QLineF& line) -> QString
    {
        const double dx = line.x2() - line.x1();
        const double dy = line.y2() - line.y1();
        const bool risesToLeft = (dx * dy) > 0.0;

        return risesToLeft
                   ? tr("Croisement Gauche\nClic-droit pour inverser")
                   : tr("Croisement Droite\nClic-droit pour inverser");
    };

    if (rc.topologyBased)
    {
        const Domain::SegmentRef overRef = crossingOverState(rc) ? rc.s2 : rc.s1;
        const auto& topo = m_model->topologySnapshot();
        const auto it = std::find_if(topo.segments.begin(),
                                     topo.segments.end(),
                                     [&overRef](const Domain::TopoSegment& segment)
                                     {
                                         return segment.ref == overRef;
                                     });

        if (it != topo.segments.end())
        {
            const QLineF overLine(static_cast<double>(it->a.xAbs),
                                  static_cast<double>(it->a.y),
                                  static_cast<double>(it->b.xAbs),
                                  static_cast<double>(it->b.y));
            return tooltipFromLine(overLine);
        }
    }
    else
    {
        const int overSegmentIndex = crossingOverState(rc) ? rc.legacySegmentB : rc.legacySegmentA;
        const auto& segments = m_model->segments();
        if (overSegmentIndex >= 0 && overSegmentIndex < static_cast<int>(segments.size()))
            return tooltipFromLine(segments[static_cast<std::size_t>(overSegmentIndex)]);
    }

    return tr("Croisement Droite\nClic-droit pour inverser");
}

bool WorkspaceScene::crossingIsModified(const RenderCrossing& rc) const
{
    if (!rc.valid)
        return false;

    if (rc.topologyBased)
    {
        const auto it = m_topologyModifiedCrossings.find(rc.key);
        return it != m_topologyModifiedCrossings.end() && it->second;
    }

    if (rc.index < 0 || rc.index >= static_cast<int>(m_legacyModifiedCrossings.size()))
        return false;

    return m_legacyModifiedCrossings[static_cast<std::size_t>(rc.index)];
}

void WorkspaceScene::markCrossingModified(const RenderCrossing& rc)
{
    if (!rc.valid)
        return;

    if (rc.topologyBased)
    {
        m_topologyModifiedCrossings[rc.key] = true;
        return;
    }

    if (rc.index < 0)
        return;

    const std::size_t crossingIndex = static_cast<std::size_t>(rc.index);
    if (m_legacyModifiedCrossings.size() <= crossingIndex)
        m_legacyModifiedCrossings.resize(crossingIndex + 1, false);

    m_legacyModifiedCrossings[crossingIndex] = true;
}

int WorkspaceScene::crossingCount() const
{
    if (!m_model)
        return 0;

    if (useTopologyCrossings())
        return static_cast<int>(m_model->topologySnapshot().crossings.size());

    return static_cast<int>(m_model->crossings().size());
}

bool WorkspaceScene::crossingIsOver(int idx, bool* ok) const
{
    const RenderCrossing rc = renderCrossingAt(idx);
    if (ok)
        *ok = rc.valid;

    if (!rc.valid)
        return false;

    return crossingOverState(rc);
}

bool WorkspaceScene::setCrossingOver(int idx, bool over)
{
    if (!m_model)
        return false;

    const RenderCrossing rc = renderCrossingAt(idx);
    if (!rc.valid)
        return false;

    const bool currentState = crossingOverState(rc);
    if (currentState == over)
        return true;

    if (rc.topologyBased)
    {
        if (!m_model->setTopologyCrossingOver(rc.key, over))
            return false;

        markCrossingModified(rc);
        update();
        return true;
    }

    const auto& crossings = m_model->crossings();
    if (idx < 0 || idx >= static_cast<int>(crossings.size()))
        return false;

    m_model->invertCrossing(static_cast<std::size_t>(idx));
    markCrossingModified(rc);

    update();
    return true;
}

bool WorkspaceScene::toggleCrossingOver(int idx)
{
    bool ok = false;
    const bool cur = crossingIsOver(idx, &ok);
    if (!ok)
        return false;

    return setCrossingOver(idx, !cur);
}

QPointF WorkspaceScene::activeCrossingAbsPosMM() const
{
    const RenderCrossing rc = renderCrossingAt(m_activeCrossingIndex);
    return rc.valid ? rc.absPosMM : QPointF();
}

void WorkspaceScene::setCrossingOverviewEnabled(bool enabled)
{
    if (m_crossingOverviewEnabled == enabled)
        return;

    m_crossingOverviewEnabled = enabled;
    clearCrossingEditMarks();

    update();
}

void WorkspaceScene::clearCrossingEditMarks()
{
    m_legacyModifiedCrossings.clear();
    m_topologyModifiedCrossings.clear();
}
int WorkspaceScene::findCrossingNear(const QPointF& worldPosMM, double radiusMM) const
{
    if (!m_model)
        return -1;

    const int n = crossingCount();
    if (n <= 0)
        return -1;

    const double r2 = radiusMM * radiusMM;

    int bestIdx = -1;
    double bestD2 = 1e100;

    for (int i = 0; i < n; ++i)
    {
        const RenderCrossing rc = renderCrossingAt(i);
        if (!rc.valid)
            continue;

        double dx = rc.absPosMM.x() - worldPosMM.x();
        const int L = m_model->ribbonLengthMM();
        if (L > 0)
            dx = wrapDelta(dx, static_cast<double>(L));

        const double dy = rc.absPosMM.y() - worldPosMM.y();
        const double d2 = dx * dx + dy * dy;

        if (d2 <= r2 && d2 < bestD2)
        {
            bestD2 = d2;
            bestIdx = i;
        }
    }

    return bestIdx;
}

void WorkspaceScene::updateHoveredCrossing(const QPointF& worldPosMM)
{
    const int idx = findCrossingNear(worldPosMM, 5.0);

    if (idx == m_hoveredCrossingIndex)
        return;

    m_hoveredCrossingIndex = idx;
    m_activeCrossingIndex = idx;

    if (idx >= 0)
        refreshHoveredCrossing();
    else
        QToolTip::hideText();

    update();
}
// -------------------------------------------------
// Background : papier + grille + vÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©ritÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â© (points/segments wrap)
// -------------------------------------------------
void WorkspaceScene::drawBackground(QPainter* p, const QRectF& rect)
{
    p->fillRect(rect, QColor(255, 250, 250));
    p->save();
    p->setRenderHint(QPainter::Antialiasing, false);

    if (m_gridVisible)
        drawGrid(p, rect);
    drawPointsAndSegments(p, rect);

    // ---------------------------------------------------------
    // Masquage visuel correct basÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â© sur position liftÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©e rÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©elle
    // ---------------------------------------------------------
    if (m_model)
    {
        const double L = static_cast<double>(m_model->ribbonLengthMM());
        const auto& pts = m_model->points();

        if (!pts.empty() && L > 0.0)
        {
            const double x0Log   = pts[0].x();
            const double targetX = rect.center().x();
            const double k0      = std::llround((targetX - x0Log) / L);

            const double xLiftStart = x0Log + k0 * L;
            const double xLiftEnd   = xLiftStart + L;

            const double transitionW = 20.0;

            // 1) Bande bleutÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â©e translucide
            QRectF transRect(xLiftEnd, rect.top(), transitionW, rect.height());
            p->fillRect(transRect.intersected(rect), QColor(170, 02, 40, 90));

            // 2) Zone morte opaque totale
            QRectF deadRect(xLiftEnd + transitionW,
                            rect.top(),
                            rect.right() - (xLiftEnd + transitionW),
                            rect.height());

            p->fillRect(deadRect.intersected(rect), QColor(255, 250, 250, 255));
        }
    }

    p->restore();
}

// -------------------------------------------------
// Foreground : halo croisement actif
// -------------------------------------------------
void WorkspaceScene::drawForeground(QPainter* p, const QRectF& rect)
{
    if (!m_model)
        return;

    const bool aaPrev = p->renderHints().testFlag(QPainter::Antialiasing);
    p->setRenderHint(QPainter::Antialiasing, true);

    const QColor modifiedFill(200, 20, 32, 92);
    QPen ringPen(QColor(255, 210, 0));
    ringPen.setCosmetic(true);
    ringPen.setWidthF(1.0);
    p->setPen(ringPen);

    const int Lmm = m_model->ribbonLengthMM();
    auto drawWrappedRing = [&](const QPointF& basePos, double radius, bool modified)
    {
        p->setBrush(modified ? QBrush(modifiedFill) : Qt::NoBrush);

        if (Lmm <= 0)
        {
            p->drawEllipse(basePos, radius, radius);
            return;
        }

        const double L = static_cast<double>(Lmm);
        const int kMin = static_cast<int>(std::floor((rect.left() - basePos.x()) / L)) - 1;
        const int kMax = static_cast<int>(std::ceil((rect.right() - basePos.x()) / L)) + 1;

        for (int k = kMin; k <= kMax; ++k)
        {
            const QPointF pos(basePos.x() + static_cast<double>(k) * L, basePos.y());
            p->drawEllipse(pos, radius, radius);
        }
    };

    if (m_crossingOverviewEnabled)
    {
        const int n = crossingCount();
        const int activeRope = m_model->activeRopeId();
        const double allR = 2.0;

        for (int i = 0; i < n; ++i)
        {
            const RenderCrossing c = renderCrossingAt(i);
            if (!c.valid)
                continue;

            if (c.topologyBased)
            {
                const int rope1 = static_cast<int>(c.s1.ropeId);
                const int rope2 = static_cast<int>(c.s2.ropeId);
                if (rope1 != activeRope && rope2 != activeRope)
                    continue;
            }

            drawWrappedRing(c.absPosMM, allR, crossingIsModified(c));
        }
    }

    const RenderCrossing rc = renderCrossingAt(m_activeCrossingIndex);
    if (rc.valid)
    {
        const double activeR = m_crossingOverviewEnabled ? 2.8 : 3.4;
        drawWrappedRing(rc.absPosMM, activeR, crossingIsModified(rc));
    }

    p->setRenderHint(QPainter::Antialiasing, aaPrev);
}

// ============================================================
// End Of File
// ============================================================


