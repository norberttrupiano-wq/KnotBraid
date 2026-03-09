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
// File        : src/ui/WorkspaceView.cpp
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#include "WorkspaceView.h"
#include "WorkspaceScene.h"
#include "Animate.h"

#include "../core/SnapEngine.h"
#include "../model/WorkspaceModel.h"
#include <QCursor>
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QMessageBox>
#include <QLinearGradient>
#include <QCursor>
#include <QToolTip>

#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <algorithm>

using Core::SnapEngine;
using Core::SnapStep;

// ------------------------------------------------------------
// Constantes UI (pixels)
// ------------------------------------------------------------
static constexpr double MM_TO_PX = 10.0;
static constexpr int BLUE_RULE_HEIGHT_PX = 24;
static constexpr int JONCTION_MARGIN_MM = 5;
static constexpr int JONCTION_MARGIN_PX = static_cast<int>(JONCTION_MARGIN_MM * MM_TO_PX);

static constexpr double FADE_WIDTH_MM   = 5.0;
static constexpr double VOID_VISIBLE_MM = 10.0;

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------
static SnapStep snapStepFromModifiers(Qt::KeyboardModifiers mods)
{
    if (mods & Qt::ControlModifier) return SnapStep::MM_10;
    if (mods & Qt::ShiftModifier)   return SnapStep::MM_5;
    return SnapStep::MM_1;
}

int WorkspaceView::normalizeMod(int v, int m)
{
    if (m <= 0) return 0;
    const int r = v % m;
    return (r < 0) ? (r + m) : r;
}

// DÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â©pliage topologique X
static std::int64_t unwrapXAbs(std::int64_t xMod, std::int64_t lastXAbs, std::int64_t L)
{
    if (L <= 0)
        return xMod;

    const std::int64_t k0 = lastXAbs / L;

    std::int64_t best = xMod + k0 * L;
    std::int64_t bestDist = std::llabs(best - lastXAbs);

    for (int dk = -1; dk <= 1; ++dk)
    {
        const std::int64_t candidate = xMod + (k0 + dk) * L;
        const std::int64_t dist = std::llabs(candidate - lastXAbs);

        if (dist < bestDist)
        {
            bestDist = dist;
            best = candidate;
        }
    }

    return best;
}

static int signedDeltaRing(int to, int from, int L)
{
    if (L <= 0) return (to - from);

    int d = (to - from) % L;
    if (d < 0) d += L;
    if (d > L / 2) d -= L;
    return d;
}

static bool isDirectionAllowed8(int dx, int dy)
{
    if (dx == 0 && dy == 0)
        return false;

    if (dx == 0) return true;
    if (dy == 0) return true;
    if (std::abs(dx) == std::abs(dy))
        return true;

    return false;
}

// ============================================================
// Construction
// ============================================================

WorkspaceView::WorkspaceView(QWidget* parent)
    : QGraphicsView(parent)
{
    m_scene = new WorkspaceScene(this);
    setScene(m_scene);

    m_anim = new Ui::Animate(this);
    connect(m_anim, &Ui::Animate::frameChanged,
            this, [this](std::size_t segIndex, double progress)
            {
                if (!m_scene)
                    return;

                // segIndex = segments COMPLETS valides (0..N)
                const bool animRunning = m_anim && m_anim->running();
                m_scene->setAnimationState(animRunning, segIndex, progress);
                if (viewport())
                    viewport()->update();
            });

    connect(m_anim, &Ui::Animate::finished,
            this, [this]()
            {
                if (m_scene)
                    m_scene->setAnimationState(false, 0, 1.0);
                if (viewport())
                    viewport()->update();
            });

    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setTransformationAnchor(QGraphicsView::NoAnchor);
    setResizeAnchor(QGraphicsView::NoAnchor);

    setRenderHint(QPainter::Antialiasing, false);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setFrameStyle(QFrame::NoFrame);
    setMouseTracking(true);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    applyHome();
}

// ============================================================
// Crossings Edit Mode (LK-STRICT)
// ============================================================

void WorkspaceView::enterCrossingEditMode()
{
    if (!m_model || !m_scene)
        return;

    m_crossingEditMode = true;
    emit crossingEditModeChanged(true);
    m_scene->setCrossingOverviewEnabled(true);

    goHome();

    const int n = m_scene->crossingCount();
    if (n <= 0)
    {
        m_activeCrossingIndex = -1;
        m_scene->setActiveCrossingIndex(-1);
        return;
    }

    setActiveCrossingIndex(0, true);

    setCursor(Qt::CrossCursor);

    m_hasPreview = false;
    m_previewValid = false;
    if (m_scene)
        m_scene->hideSnapHalo();
    QToolTip::hideText();
}

void WorkspaceView::exitCrossingEditMode()
{
    m_crossingEditMode = false;
    emit crossingEditModeChanged(false);
    if (m_scene)
        m_scene->setCrossingOverviewEnabled(false);
    m_activeCrossingIndex = -1;

    if (m_scene)
        m_scene->setActiveCrossingIndex(-1);

    unsetCursor();
    QToolTip::hideText();
    if (viewport())
        viewport()->update();
}

void WorkspaceView::goHome()
{
    if (!m_model)
        return;

    m_model->setRibbonOffsetMM(0);
    SnapEngine::setRibbonOffset(m_model->ribbonOffsetMM());
    updateViewTransform();

    m_hasLastSnapXAbs = false;

    if (m_scene)
        m_scene->hideSnapHalo();

    if (viewport())
        viewport()->update();
}

void WorkspaceView::setActiveCrossingIndex(int idx, bool recenter)
{
    if (!m_model || !m_scene)
        return;

    const int n = m_scene->crossingCount();
    if (n <= 0)
        return;

    if (idx < 0)  idx = n - 1;
    if (idx >= n) idx = 0;

    m_activeCrossingIndex = idx;
    m_scene->setActiveCrossingIndex(idx);

    if (recenter)
        ensureActiveCrossingVisibleX();

    if (m_scene)
        m_scene->refreshActiveCrossingTooltipAt(activeCrossingGlobalTooltipPos());

    warpCursorToActiveCrossing();

    if (viewport())
        viewport()->update();
}

QPoint WorkspaceView::activeCrossingGlobalTooltipPos() const
{
    if (!m_scene)
        return QCursor::pos();

    if (m_scene->activeCrossingIndex() < 0)
        return QCursor::pos();

    const QPointF absPos = m_scene->activeCrossingAbsPosMM();
    const QPoint vp = mapFromScene(absPos);
    return viewport()->mapToGlobal(vp);
}

void WorkspaceView::warpCursorToActiveCrossing()
{
    if (!m_crossingEditMode)
        return;

    QCursor::setPos(activeCrossingGlobalTooltipPos());
}

// ============================================================
// Model
// ============================================================

void WorkspaceView::setModel(Model::WorkspaceModel* model)
{
    m_model = model;

    if (m_scene)
        m_scene->setModel(model);

    syncFromModel();
}

void WorkspaceView::syncFromModel()
{
    if (m_model)
    {
        SnapEngine::setRibbonLength(m_model->ribbonLengthMM());
        SnapEngine::setRibbonOffset(m_model->ribbonOffsetMM());
    }

    updateViewTransform();

    if (scene())
        scene()->update();
    if (viewport())
        viewport()->update();

    m_hasPreview = false;
    m_hasLastSnapXAbs = false;
    m_lastSnapXAbs = 0;
}

// ============================================================
// Animation
// ============================================================

void WorkspaceView::startAnimationSimple()
{
    if (!m_model || !m_anim || !m_scene)
        return;

    const auto& topo = m_model->topologySnapshot();
    std::size_t totalSegments = topo.segments.size();

    // Fallback legacy si le snapshot topo est vide.
    if (totalSegments == 0)
    {
        const auto& pts = m_model->points();
        totalSegments = (pts.size() >= 2) ? (pts.size() - 1) : 0;
    }

    m_anim->stop();
    m_anim->setTotalSegments(totalSegments);
    m_anim->setMode(Ui::Animate::Mode::Simple);
    m_anim->rewind();

    // IMPORTANT : dÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â©marre en mode animÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â©
    m_scene->setAnimationState(true, 0, 0.0);
    m_anim->start();
}

void WorkspaceView::startAnimationSerpent()
{
    if (!m_model || !m_anim || !m_scene)
        return;

    const auto& topo = m_model->topologySnapshot();
    std::size_t totalSegments = topo.segments.size();

    // Fallback legacy si le snapshot topo est vide.
    if (totalSegments == 0)
    {
        const auto& pts = m_model->points();
        totalSegments = (pts.size() >= 2) ? (pts.size() - 1) : 0;
    }

    m_anim->stop();
    m_anim->setTotalSegments(totalSegments);
    m_anim->setMode(Ui::Animate::Mode::Serpent);
    m_anim->rewind();

    m_scene->setAnimationState(true, 0, 0.0);
    m_anim->start();
}

void WorkspaceView::startAnimation()
{
    // LK-STRICT : "Play Animation" = Serpent par dÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â©faut
    startAnimationSerpent();
}

void WorkspaceView::stopAnimation()
{
    if (!m_anim || !m_scene)
        return;

    m_anim->stop();
    m_scene->setAnimationState(false, 0, 1.0);
    if (viewport())
        viewport()->update();
}

bool WorkspaceView::isAnimating() const
{
    return m_anim && m_anim->running();
}

// ============================================================
// Transform UNIQUE
// ============================================================

void WorkspaceView::applyHome()
{
    updateViewTransform();
}

void WorkspaceView::updateViewTransform()
{
    const int offsetMM = m_model ? m_model->ribbonOffsetMM() : 0;

    QTransform t;
    t.translate(JONCTION_MARGIN_PX, BLUE_RULE_HEIGHT_PX);
    t.scale(MM_TO_PX, MM_TO_PX);
    t.translate(-static_cast<double>(offsetMM), 0.0);

    setTransform(t);
}

// ============================================================
// FrontiÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¨re honnÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Âªte pixel ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ monde
// ============================================================

QPointF WorkspaceView::interpretPixelAsWorld(const QPoint& vp) const
{
    QPoint v = vp;
    if (v.y() < BLUE_RULE_HEIGHT_PX)
        v.setY(BLUE_RULE_HEIGHT_PX);

    return mapToScene(v);
}

// ============================================================
// Input
// ============================================================
// void WorkspaceView::mousePressEvent(QMouseEvent* e)
// {
//     if (m_crossingEditMode)
//     {
//         e->accept();
//         return;
//     }

//     if (!m_model || e->button() != Qt::LeftButton)
//     {
//         QGraphicsView::mousePressEvent(e);
//         return;
//     }

//     const QPointF worldUnwrapped = interpretPixelAsWorld(e->pos());
//     const SnapStep step = snapStepFromModifiers(e->modifiers());

//     const int L = m_model->ribbonLengthMM();

//     const double stepMM =
//         (step == SnapStep::MM_10) ? 10.0 :
//             (step == SnapStep::MM_5)  ? 5.0  : 1.0;

//     const int yI = static_cast<int>(
//         std::llround(std::round(worldUnwrapped.y() / stepMM) * stepMM));

//     const std::int64_t xMod = static_cast<std::int64_t>(
//         std::llround(std::round(worldUnwrapped.x() / stepMM) * stepMM));

//     std::int64_t xAbs = m_hasLastSnapXAbs ? unwrapXAbs(xMod, m_lastSnapXAbs, L) : xMod;

//     m_lastSnapXAbs = xAbs;
//     m_hasLastSnapXAbs = true;

//     const int logicalX = normalizeMod(static_cast<int>(xAbs), L);

//     const QPointF snapLogical(static_cast<double>(logicalX),
//                               static_cast<double>(yI));

//     // =========================================================
//     // MODE ESQUISSE
//     // =========================================================
//     if (m_sketchMode)
//     {
//         QPointF p = snapLogical;

//         // premier point
//         if (m_sketchPoints.empty())
//         {
//             m_sketchPoints.push_back(p);
//             viewport()->update();
//             return;
//         }

//         QPointF last = m_sketchPoints.back();

//         int dx = static_cast<int>(std::llround(p.x())) -
//                  static_cast<int>(std::llround(last.x()));

//         int dy = static_cast<int>(std::llround(p.y())) -
//                  static_cast<int>(std::llround(last.y()));

//         if (dx == 0 && dy == 0)
//             return;

//         if (!isDirectionAllowed8(dx, dy))
//             return;

//         m_sketchPoints.push_back(p);

//         if (!m_sketchBreak)
//         {
//             QLineF seg(last, p);
//             m_sketchSegments.push_back(seg);
//         }

//         m_sketchBreak = false;

//         viewport()->update();
//         return;
//     }

//     // =========================================================
//     // TRACE NORMAL (points bleus)
//     // =========================================================

//     bool allow = true;
//     const auto& pts = m_model->points();

//     if (!pts.empty())
//     {
//         const QPointF last = pts.back();

//         const int lastX = static_cast<int>(std::llround(last.x()));
//         const int lastY = static_cast<int>(std::llround(last.y()));

//         const int dx = signedDeltaRing(logicalX, lastX, L);
//         const int dy = (yI - lastY);

//         allow = isDirectionAllowed8(dx, dy);
//     }

//     if (m_scene)
//         m_scene->updateSnapHalo(QPointF(static_cast<double>(xAbs),
//                                         static_cast<double>(yI)));

//     emit mousePositionChanged(snapLogical);

//     if (!allow)
//     {
//         e->accept();
//         return;
//     }

//     if (m_scene)
//         m_scene->addPoint(snapLogical);

//     QGraphicsView::mousePressEvent(e);
// }
// ------------------------------------------------------------
// WorkspaceView::mousePressEvent
// MODE ESQUISSE corrigÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â© (wrap cylindre + X absolu)
// ------------------------------------------------------------
void WorkspaceView::mousePressEvent(QMouseEvent* e)
{
    if (!m_model)
    {
        QGraphicsView::mousePressEvent(e);
        return;
    }

    if (m_crossingEditMode)
    {
        if (e->button() == Qt::RightButton && m_scene)
        {
            const QPointF worldPos = interpretPixelAsWorld(e->pos());
            m_scene->updateHoveredCrossing(worldPos);

            const int idx = m_scene->hoveredCrossingIndex();
            if (idx >= 0 && m_scene->toggleCrossingOver(idx))
            {
                m_scene->refreshHoveredCrossing();
                if (viewport())
                    viewport()->update();
                e->accept();
                return;
            }
        }

        e->accept();
        return;
    }

    if (e->button() != Qt::LeftButton)
    {
        QGraphicsView::mousePressEvent(e);
        return;
    }

    // --------------------------------------------------------
    // CoordonnÃƒÆ’Ã‚Â©es souris
    // --------------------------------------------------------
    const QPointF scenePos = interpretPixelAsWorld(e->pos());

    int xI = static_cast<int>(std::llround(scenePos.x()));
    int yI = static_cast<int>(std::llround(scenePos.y()));

    int snapStep = 1;
    if (e->modifiers() & Qt::ShiftModifier)  snapStep = 5;
    if (e->modifiers() & Qt::ControlModifier) snapStep = 10;

    xI = (xI / snapStep) * snapStep;
    yI = (yI / snapStep) * snapStep;

    const int L = m_model->ribbonLengthMM();

    int xLogical = ((xI % L) + L) % L;
    int xAbs     = xI;

    QPointF snapLogical(static_cast<double>(xLogical),
                        static_cast<double>(yI));

    // --------------------------------------------------------
    // MODE ESQUISSE
    // --------------------------------------------------------
    if (m_sketchMode)
    {
        QPointF p(static_cast<double>(xAbs),
                  static_cast<double>(yI));

        if (m_sketchPoints.empty())
        {
            m_sketchPoints.push_back(p);
        }
        else
        {
            const QPointF& last = m_sketchPoints.back();

            const int px = static_cast<int>(std::llround(p.x()));
            const int lx = static_cast<int>(std::llround(last.x()));

            int dx = signedDeltaRing(px, lx, L);
            int dy = static_cast<int>(std::llround(p.y())) -
                     static_cast<int>(std::llround(last.y()));

            QPointF corrected(last.x() + dx,
                              last.y() + dy);

            m_sketchPoints.push_back(corrected);
        }

        viewport()->update();
        return;
    }

    // --------------------------------------------------------
    // MODE POINT
    // --------------------------------------------------------
    bool allow = true;
    const auto& topo = m_model->topologySnapshot();
    const int activeRope = std::clamp(
        m_model->activeRopeId(),
        0,
        static_cast<int>(Domain::MaxRopes) - 1);

    if (activeRope >= 0 && activeRope < static_cast<int>(topo.ropes.size()))
    {
        const auto& ropePoints = topo.ropes[static_cast<std::size_t>(activeRope)].points;
        if (!ropePoints.empty())
        {
            const auto& lastPoint = ropePoints.back();
            const int lastXLogical = normalizeMod(static_cast<int>(lastPoint.xAbs), L);
            const int lastY = static_cast<int>(lastPoint.y);

            const int dx = signedDeltaRing(xLogical, lastXLogical, L);
            const int dy = yI - lastY;
            allow = isDirectionAllowed8(dx, dy);
        }
    }
    else
    {
        // MIGRATION-PARALLEL: fallback legacy en cas d'etat rope incomplet.
        const auto& pts = m_model->points();
        if (!pts.empty())
        {
            const QPointF last = pts.back();
            const int lastXLogical = static_cast<int>(std::llround(last.x()));
            const int lastY = static_cast<int>(std::llround(last.y()));
            const int dx = signedDeltaRing(xLogical, lastXLogical, L);
            const int dy = yI - lastY;
            allow = isDirectionAllowed8(dx, dy);
        }
    }

    if (!allow)
    {
        e->accept();
        return;
    }

    m_model->addPoint(snapLogical);

    // Keep mouse-move unwrapping anchored to the actual last point of the active rope.
    const auto& topoAfterAdd = m_model->topologySnapshot();
    if (activeRope >= 0 && activeRope < static_cast<int>(topoAfterAdd.ropes.size()))
    {
        const auto& ropePointsAfterAdd = topoAfterAdd.ropes[static_cast<std::size_t>(activeRope)].points;
        if (!ropePointsAfterAdd.empty())
        {
            m_lastSnapXAbs = ropePointsAfterAdd.back().xAbs;
            m_hasLastSnapXAbs = true;
        }
    }

    viewport()->update();
}

void WorkspaceView::mouseMoveEvent(QMouseEvent* e)
{
    if (m_crossingEditMode)
    {
        m_hasPreview = false;
        m_previewValid = false;

        if (m_scene)
        {
            const QPointF worldPos = interpretPixelAsWorld(e->pos());
            m_scene->updateHoveredCrossing(worldPos);
            m_scene->hideSnapHalo();
        }

        if (viewport())
            viewport()->update();

        e->accept();
        return;
    }
    if (m_sketchMode)
    {
        m_hasPreview = false;

        if (m_scene)
            m_scene->hideSnapHalo();

        QGraphicsView::mouseMoveEvent(e);
        return;
    }

    if (!m_model)
    {
        QGraphicsView::mouseMoveEvent(e);
        return;
    }

    const QPointF scenePos = interpretPixelAsWorld(e->pos());
    int xI = static_cast<int>(std::llround(scenePos.x()));
    int yI = static_cast<int>(std::llround(scenePos.y()));

    int snapStep = 1;
    if (e->modifiers() & Qt::ShiftModifier)  snapStep = 5;
    if (e->modifiers() & Qt::ControlModifier) snapStep = 10;

    xI = (xI / snapStep) * snapStep;
    yI = (yI / snapStep) * snapStep;

    const int L = m_model->ribbonLengthMM();

    bool hasLastPoint = false;
    std::int64_t lastXAbs = 0;
    int lastXLogical = 0;
    int lastY = 0;

    const auto& topo = m_model->topologySnapshot();
    const int activeRope = std::clamp(
        m_model->activeRopeId(),
        0,
        static_cast<int>(Domain::MaxRopes) - 1);

    if (activeRope >= 0 && activeRope < static_cast<int>(topo.ropes.size()))
    {
        const auto& ropePoints = topo.ropes[static_cast<std::size_t>(activeRope)].points;
        if (!ropePoints.empty())
        {
            const auto& lastPoint = ropePoints.back();
            lastXAbs = lastPoint.xAbs;
            lastXLogical = normalizeMod(static_cast<int>(lastPoint.xAbs), L);
            lastY = static_cast<int>(lastPoint.y);
            hasLastPoint = true;
        }
    }

    if (!hasLastPoint)
    {
        // MIGRATION-PARALLEL: fallback legacy en cas d'etat rope incomplet.
        const auto& pts = m_model->points();
        if (!pts.empty())
        {
            const QPointF lastLogical = pts.back();
            lastXLogical = static_cast<int>(std::llround(lastLogical.x()));
            lastY = static_cast<int>(std::llround(lastLogical.y()));
            lastXAbs = static_cast<std::int64_t>(lastXLogical);
            hasLastPoint = true;
        }
    }

    int logicalX = normalizeMod(xI, L);
    const std::int64_t xDisplayAbs = static_cast<std::int64_t>(xI);
    std::int64_t xTopoAbs = xDisplayAbs;
    int dxToLast = 0;

    if (hasLastPoint)
    {
        dxToLast = signedDeltaRing(logicalX, lastXLogical, L);
        xTopoAbs = lastXAbs + static_cast<std::int64_t>(dxToLast);
        logicalX = normalizeMod(static_cast<int>(xTopoAbs), L);
    }

    m_lastSnapXAbs = xTopoAbs;
    m_hasLastSnapXAbs = true;

    const QPointF snapLogical(static_cast<double>(logicalX), static_cast<double>(yI));

    if (m_scene)
    {
        const QPointF hoverPos(static_cast<double>(xDisplayAbs), static_cast<double>(yI));
        m_scene->updateHoveredCrossing(hoverPos);

        m_scene->updateSnapHalo(hoverPos);
    }

    if (hasLastPoint)
    {
        const int dx = dxToLast;
        const int dy = yI - lastY;

        if (dx == 0 && dy == 0)
        {
            m_hasPreview = false;
        }
        else
        {
            m_previewValid = isDirectionAllowed8(dx, dy);
            m_hasPreview = true;
            const std::int64_t lastXDisplayAbs = xDisplayAbs - static_cast<std::int64_t>(dx);
            m_previewStart = QPointF(static_cast<double>(lastXDisplayAbs), static_cast<double>(lastY));
            m_previewEnd = QPointF(static_cast<double>(xDisplayAbs), static_cast<double>(yI));
        }
    }
    else
    {
        m_hasPreview = false;
    }

    emit mousePositionChanged(snapLogical);

    viewport()->update();
    QGraphicsView::mouseMoveEvent(e);
}

void WorkspaceView::mouseReleaseEvent(QMouseEvent* e)
{
    QGraphicsView::mouseReleaseEvent(e);
}

//-------------------
// clavier
//-------------------
void WorkspaceView::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Insert)
    {
        if (m_crossingEditMode)
            exitCrossingEditMode();
        else
            enterCrossingEditMode();

        e->accept();
        return;
    }

    if (m_crossingEditMode && e->key() == Qt::Key_Escape)
    {
        exitCrossingEditMode();
        e->accept();
        return;
    }
    if (m_sketchMode && e->key() == Qt::Key_Space)
    {
        m_sketchBreak = true;
        return;
    }

    if (e->key() == Qt::Key_Home)
    {
        goHome();
        e->accept();
        return;
    }

    if (!m_model)
    {
        QGraphicsView::keyPressEvent(e);
        return;
    }

    if (e->modifiers() & Qt::AltModifier)
    {
        if (m_model->wrapIsDone())
        {
            QMessageBox::warning(this, tr("Ruban verrouill\u00E9"),
                                 tr("La longueur du ruban doit \u00EAtre d\u00E9finie avant le passage de la jonction.\n\n"
                                    "Utilisez Ctrl+Z pour revenir en arri\u00E8re."));
            return;
        }

        int step = 1;

        if ((e->modifiers() & Qt::ControlModifier) && (e->modifiers() & Qt::ShiftModifier))
            step = 10;
        else if (e->modifiers() & Qt::ControlModifier)
            step = 5;

        if (e->key() == Qt::Key_Right)
        {
            m_model->resizeRibbonMM(step);
            scene()->update();
            return;
        }

        if (e->key() == Qt::Key_Left)
        {
            m_model->resizeRibbonMM(-step);
            scene()->update();
            return;
        }
    }

    if (e->modifiers() & Qt::ControlModifier)
    {
        if (e->key() == Qt::Key_Z)
        {
            if (m_model->canUndo())
            {
                m_model->undo();
                if (m_scene)
                    m_scene->update();
                viewport()->update();
            }
            e->accept();
            return;
        }

        if (e->key() == Qt::Key_Y)
        {
            if (m_model->canRedo())
            {
                m_model->redo();
                if (m_scene)
                    m_scene->update();
                viewport()->update();
            }
            e->accept();
            return;
        }
    }

    const int step =
        (e->modifiers() & Qt::ControlModifier) ? 10 :
            (e->modifiers() & Qt::ShiftModifier)   ? 5  : 1;

    if (e->key() == Qt::Key_Left)
    {
        m_model->rotateRibbonMM(-step);
        SnapEngine::setRibbonOffset(m_model->ribbonOffsetMM());
        updateViewTransform();
        m_hasLastSnapXAbs = false;
        if (m_scene)
            m_scene->update();
        viewport()->update();
        e->accept();
        return;
    }

    if (e->key() == Qt::Key_Right)
    {
        m_model->rotateRibbonMM(step);
        SnapEngine::setRibbonOffset(m_model->ribbonOffsetMM());
        updateViewTransform();
        m_hasLastSnapXAbs = false;
        if (m_scene)
            m_scene->update();
        viewport()->update();
        e->accept();
        return;
    }

    QGraphicsView::keyPressEvent(e);
}

void WorkspaceView::wheelEvent(QWheelEvent* e)
{
    QGraphicsView::wheelEvent(e);
}

void WorkspaceView::resizeEvent(QResizeEvent* e)
{
    QGraphicsView::resizeEvent(e);
    updateViewTransform();
    update();
}

// ============================================================
// Paint (scÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¨ne + overlays)
// ============================================================

void WorkspaceView::paintEvent(QPaintEvent* e)
{
    QGraphicsView::paintEvent(e);

    QPainter p(viewport());
    p.setRenderHint(QPainter::Antialiasing, false);
    p.resetTransform();

    // --------------------------------------------
    // Overlays systÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¨me
    // --------------------------------------------
    drawBlueRuleOverlay(&p);
    drawJonctionOverlay(&p);
    drawVoidOverlay(&p);

    // --------------------------------------------
    // Esquisse (segments gris)
    // --------------------------------------------
    if (!m_sketchSegments.empty())
    {
        QPen pen(QColor(120,120,120,255));
        pen.setWidth(6);
        pen.setStyle(Qt::SolidLine);
        pen.setCapStyle(Qt::RoundCap);

        p.setPen(pen);

        for (const auto& s : m_sketchSegments)
        {
            QPointF a = mapFromScene(s.p1());
            QPointF b = mapFromScene(s.p2());

            p.drawLine(a, b);
        }
    }
    // --------------------------------------------
    // Points de l'esquisse
    // --------------------------------------------
    if (!m_sketchPoints.empty())
    {
        QBrush brush(QColor(0,255,0,255));
        p.setBrush(brush);
        p.setPen(Qt::NoPen);

        const double r = 4.0;   // rayon plus petit que point bleu

        for (const auto& pt : m_sketchPoints)
        {
            QPointF px = mapFromScene(pt);
            p.drawEllipse(px, r, r);
        }
    }
    // --------------------------------------------
    // Preview segment (points bleus)
    // --------------------------------------------
    if (m_hasPreview)
    {
        QPainter p2(viewport());
        p2.setRenderHint(QPainter::Antialiasing, true);

        const QPointF a = mapFromScene(m_previewStart);
        const QPointF b = mapFromScene(m_previewEnd);

        QPen pen(m_previewValid ? QColor(0, 200, 0) : QColor(200, 0, 0));
        pen.setWidth(2);
        pen.setCapStyle(Qt::RoundCap);

        p2.setPen(pen);
        p2.drawLine(a, b);
    }
}
// ============================================================
// Overlays (inchangÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â©s - repris de ta version)
// ============================================================

void WorkspaceView::drawBlueRuleOverlay(QPainter* p)
{
    const QRect r(0, 0, viewport()->width(), BLUE_RULE_HEIGHT_PX);
    p->fillRect(r, QColor(60, 90, 200));

    const int L = m_model ? m_model->ribbonLengthMM() : 280;
    const int offsetMM = m_model ? m_model->ribbonOffsetMM() : 0;

    const QPen pen1 (QColor(210, 225, 255));
    const QPen pen5 (QColor(190, 215, 255));
    const QPen pen10(QColor(170, 205, 255));

    const int leftPx  = 0;
    const int rightPx = viewport()->width();
    const int h       = BLUE_RULE_HEIGHT_PX;

    const double sceneXLeft  = static_cast<double>(offsetMM) + (leftPx  - JONCTION_MARGIN_PX) / MM_TO_PX;
    const double sceneXRight = static_cast<double>(offsetMM) + (rightPx - JONCTION_MARGIN_PX) / MM_TO_PX;

    const int mmStart = static_cast<int>(std::floor(sceneXLeft));
    const int mmEnd   = static_cast<int>(std::ceil (sceneXRight));

    for (int sceneMM = mmStart; sceneMM <= mmEnd; ++sceneMM)
    {
        const int x = static_cast<int>(std::round(
            JONCTION_MARGIN_PX + (static_cast<double>(sceneMM - offsetMM) * MM_TO_PX)));

        const int label = normalizeMod(sceneMM, L);

        if (label % 10 == 0)
        {
            p->setPen(pen10);
            p->drawLine(x, h, x, h - 10);
            p->drawText(x + 2, h - 10, QString::number(label));
        }
        else if (label % 5 == 0)
        {
            p->setPen(pen5);
            p->drawLine(x, h, x, h - 7);
        }
        else
        {
            p->setPen(pen1);
            p->drawLine(x, h, x, h - 4);
        }
    }
}

void WorkspaceView::drawJonctionOverlay(QPainter* p)
{
    const int L = m_model ? m_model->ribbonLengthMM() : 280;
    const int offsetMM = m_model ? m_model->ribbonOffsetMM() : 0;

    const double Lpx  = static_cast<double>(L) * MM_TO_PX;
    const double base = static_cast<double>(JONCTION_MARGIN_PX)
                        - static_cast<double>(offsetMM) * MM_TO_PX;

    QPen seamPen(QColor(0, 200, 0, 180));
    seamPen.setWidth(6);
    p->setPen(seamPen);

    const int leftPx  = 0;
    const int rightPx = viewport()->width();

    QRectF sceneRect = mapToScene(viewport()->rect()).boundingRect();
    const double worldBottomMM = sceneRect.bottom();

    const int top = BLUE_RULE_HEIGHT_PX;
    const int bottom = static_cast<int>(BLUE_RULE_HEIGHT_PX + worldBottomMM * MM_TO_PX);

    int k = 0;
    if (Lpx > 0.0)
        k = static_cast<int>(std::floor((leftPx - base) / Lpx)) - 1;

    for (;; ++k)
    {
        const double xD = base + static_cast<double>(k) * Lpx;
        if (xD > rightPx + Lpx) break;

        const int x = static_cast<int>(std::round(xD));
        if (x >= leftPx && x <= rightPx)
            p->drawLine(x, top, x, bottom);
    }
}

void WorkspaceView::drawVoidOverlay(QPainter* p)
{
    const double fadeWidthPx   = FADE_WIDTH_MM * MM_TO_PX;

    const int L = m_model ? m_model->ribbonLengthMM() : 280;

    const double xEndPx =
        static_cast<double>(JONCTION_MARGIN_PX) +
        static_cast<double>(L) * 0.5 * MM_TO_PX;

    const double xVoidStartPx = xEndPx + fadeWidthPx;

    const int top    = BLUE_RULE_HEIGHT_PX;
    const int bottom = viewport()->height();
    const int h      = bottom - top;
    if (h <= 0) return;

    {
        QLinearGradient leftGrad(0.0, 0.0, fadeWidthPx, 0.0);
        leftGrad.setColorAt(0.0, QColor(60, 0, 90, 160));
        leftGrad.setColorAt(1.0, QColor(160, 80, 200, 50));
        p->fillRect(QRectF(0.0, top, fadeWidthPx, h), leftGrad);
    }

    {
        QLinearGradient rightGrad(xEndPx, 0.0, xEndPx + fadeWidthPx, 0.0);
        rightGrad.setColorAt(0.0, QColor(160, 80, 200, 50));
        rightGrad.setColorAt(1.0, QColor(60, 0, 90, 160));
        p->fillRect(QRectF(xEndPx, top, fadeWidthPx, h), rightGrad);
    }

    {
        const double x0 = xVoidStartPx;
        const double x1 = static_cast<double>(viewport()->width());
        if (x1 <= x0) return;

        p->fillRect(QRectF(x0, top, x1 - x0, h), QColor(8, 8, 30, 255));
    }
}

void WorkspaceView::ensureActiveCrossingVisibleX()
{
    if (!m_model || !m_scene || m_activeCrossingIndex < 0 || !viewport())
        return;

    const auto& crossings = m_model->crossings();
    if (m_activeCrossingIndex >= static_cast<int>(crossings.size()))
        return;

    const int L = m_model->ribbonLengthMM();
    if (L <= 0)
        return;

    const auto& c = crossings[static_cast<std::size_t>(m_activeCrossingIndex)];
    const double xAbs = c.positionMM.x() + static_cast<double>(c.tour) * static_cast<double>(L);

    const QPoint vpL(0, BLUE_RULE_HEIGHT_PX + 2);
    const QPoint vpR(viewport()->width() - 1, BLUE_RULE_HEIGHT_PX + 2);

    const double leftAbs  = mapToScene(vpL).x();
    const double rightAbs = mapToScene(vpR).x();

    if (xAbs >= leftAbs && xAbs <= rightAbs)
        return;

    const int step = 10;
    int guard = 0;

    while (guard++ < 200)
    {
        const QPointF curL = mapToScene(vpL);
        const QPointF curR = mapToScene(vpR);

        if (xAbs < curL.x())
            m_model->rotateRibbonMM(-step);
        else if (xAbs > curR.x())
            m_model->rotateRibbonMM(step);
        else
            break;

        SnapEngine::setRibbonOffset(m_model->ribbonOffsetMM());
        updateViewTransform();
    }

    m_hasLastSnapXAbs = false;
    if (viewport())
        viewport()->update();
}
// ------------------------------------------------------------
// Mode Esquisse
// ------------------------------------------------------------
void WorkspaceView::enterSketchMode()
{
    m_sketchMode = true;
    QPixmap pix("C:/Windows/Cursors/pen.cur");
    QCursor penCursor(pix);
    setCursor(penCursor);
    // couper le traceur normal
    m_hasPreview = false;

    if (m_scene)
        m_scene->hideSnapHalo();

    viewport()->update();
}
// ------------------------------------------------------------
// Sectionner esquisse
// ------------------------------------------------------------
void WorkspaceView::breakSketch()
{
    if (!m_sketchMode)
        return;

    //m_sketchDrawing = false;
}
// ------------------------------------------------------------
// Mode TraÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â§age
// ------------------------------------------------------------
void WorkspaceView::enterTracingMode()
{
    unsetCursor();
    m_sketchMode = false;
}


// ============================================================
// End Of File
// ============================================================
