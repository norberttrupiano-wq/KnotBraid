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
// File        : src/ui/WorkspaceScene.h
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#pragma once

#include <QColor>
#include <QGraphicsScene>
#include <QPointF>

#include <cstddef>
#include <map>
#include <vector>

#include "../domain/TopologyTypes.h"

class SnapHaloItem;

namespace Model
{
class WorkspaceModel;
struct Crossing;
}

class WorkspaceScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit WorkspaceScene(QObject* parent = nullptr);

    void setModel(Model::WorkspaceModel* model);

    // Animation
    void setAnimationState(bool enabled, std::size_t completedSegments, double progress);

    // Grid visibility (print profile)
    void setGridVisible(bool visible);
    bool isGridVisible() const { return m_gridVisible; }

    // Snap halo
    void updateSnapHalo(const QPointF& posMM);
    void hideSnapHalo();

    // Model truth
    void addPoint(const QPointF& posMM);

    // Crossings (hover + edit mode)
    void updateHoveredCrossing(const QPointF& worldPosMM);
    int hoveredCrossingIndex() const { return m_hoveredCrossingIndex; }

    void setActiveCrossingIndex(int idx) { m_activeCrossingIndex = idx; }
    int activeCrossingIndex() const { return m_activeCrossingIndex; }
    void setCrossingOverviewEnabled(bool enabled);

    QPointF activeCrossingAbsPosMM() const;

    void refreshHoveredCrossing();
    void refreshActiveCrossingTooltip();
    void refreshActiveCrossingTooltipAt(const QPoint& globalPos);

    int crossingCount() const;
    bool crossingIsOver(int idx, bool* ok = nullptr) const;
    bool setCrossingOver(int idx, bool over);
    bool toggleCrossingOver(int idx);

protected:
    void drawBackground(QPainter* p, const QRectF& rect) override;
    void drawForeground(QPainter* p, const QRectF& rect) override;

private:
    // Rendering
    void drawGrid(QPainter* p, const QRectF& rect);
    void drawPointsAndSegments(QPainter* p, const QRectF& rect);
    void drawPointsAndSegmentsFromTopology(QPainter* p, const QRectF& rect);

    // Crossing helpers
    struct RenderCrossing
    {
        bool valid = false;
        bool topologyBased = false;
        int index = -1;

        QPointF absPosMM;
        int tour = 0;

        Domain::SegmentRef s1;
        Domain::SegmentRef s2;
        bool s2OverS1 = true;
        Domain::CrossingKey key;

        int legacySegmentA = -1;
        int legacySegmentB = -1;
        bool legacyNewSegmentOver = true;
    };

    bool useTopologyCrossings() const;
    RenderCrossing renderCrossingAt(int idx) const;
    QColor ropeColor(Domain::RopeId ropeId) const;
    bool crossingOverState(const RenderCrossing& rc) const;

    int findCrossingNear(const QPointF& worldPosMM, double radiusMM) const;

    // Legacy perf cache: crossings indexed by segmentBIndex
    void rebuildCrossingsCacheIfNeeded(std::size_t totalSegments);

private:
    Model::WorkspaceModel* m_model = nullptr;

    SnapHaloItem* m_snapHalo = nullptr;
    QPointF m_lastSnapPosMM;
    bool m_hasLastSnap = false;

    // Hover / Active crossing
    int m_hoveredCrossingIndex = -1;
    int m_activeCrossingIndex = -1;
    bool m_crossingOverviewEnabled = false;

    // Animation state
    bool m_animEnabled = false;
    std::size_t m_animCompletedSegments = 0;
    double m_animProgress = 1.0;
    bool m_gridVisible = true;

    // Legacy perf cache
    std::size_t m_cacheSegmentsCount = 0;
    std::size_t m_cacheCrossingsCount = 0;
    std::vector<std::vector<const Model::Crossing*>> m_crossingsBySegB;

    // Rendering-layer override for topology crossing over/under
    std::map<Domain::CrossingKey, bool> m_topologyOverByKey;
};

// ============================================================
// End Of File
// ============================================================
