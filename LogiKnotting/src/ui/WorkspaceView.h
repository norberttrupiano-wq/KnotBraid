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
// File        : src/ui/WorkspaceView.h
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#pragma once

#include <QGraphicsView>
#include <QPointF>
#include <QLineF>
#include <cstdint>
#include <vector>

namespace Ui { class Animate; }

class WorkspaceScene;
namespace Model { class WorkspaceModel; }

class WorkspaceView : public QGraphicsView
{
    Q_OBJECT

    // ------------------------------------------------------------
    // Modes Esquisse / Traçage
    // ------------------------------------------------------------
public slots:
    void enterSketchMode();
    void breakSketch();
    void enterTracingMode();

private:
    bool m_sketchMode = false;

    // --------------------------------------------------------
    // Esquisse = clics snapés (pas de "crayon libre")
    // - 1er clic : pose l’ancre
    // - clics suivants : segment (ancre -> point) puis nouvelle ancre
    // - Space : sectionner (perd l’ancre, on repart ailleurs)
    // --------------------------------------------------------
    bool                    m_sketchHasAnchor = false;
    QPointF                 m_sketchAnchorMM;
    std::vector<QLineF>     m_sketchSegments;
    std::vector<QPointF>    m_sketchPoints;   // optionnel (trace/diagnostic)

public:
    explicit WorkspaceView(QWidget* parent = nullptr);
    void setModel(Model::WorkspaceModel* model);

    // --------------------------------------------------------
    // Animation
    // --------------------------------------------------------
    void startAnimationSimple();
    void startAnimationSerpent();
    void stopAnimation();
    bool isAnimating() const;

    // Menu "Play Animation" (LK-STRICT) :
    // - lance Serpent par défaut (pédagogique + joli)
    void startAnimation();

    // --------------------------------------------------------
    // Sync UI state from the model (ruban/jonction + redraw).
    // IMPORTANT: must be called after a successful loadFromFile().
    // --------------------------------------------------------
    void syncFromModel();
    void goHome();
    void ensureActiveCrossingVisibleX();
    bool isCrossingEditMode() const { return m_crossingEditMode; }

signals:
    void mousePositionChanged(const QPointF& posMM);
    void crossingEditModeChanged(bool enabled);

protected:
    void paintEvent(QPaintEvent* e) override;

    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

    void keyPressEvent(QKeyEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

private:
    QPointF interpretPixelAsWorld(const QPoint& vp) const;

    QPointF m_previewStart;
    QPointF m_previewEnd;
    bool    m_previewValid = false;
    bool    m_hasPreview   = false;
 //   std::vector<QPointF> m_sketchPoints;
 //   std::vector<QLineF>  m_sketchSegments;
    bool m_sketchBreak = false;
//    bool m_sketchMode  = false;
    //========================================================
    // Mode Crossings (LK-STRICT) — Touche Insert
    //========================================================
    bool m_crossingEditMode = false;
    int  m_activeCrossingIndex = -1;

    void enterCrossingEditMode();
    void exitCrossingEditMode();
    void setActiveCrossingIndex(int idx, bool recenter);
    void centerOnActiveCrossingX();
    void warpCursorToActiveCrossing();
    QPoint activeCrossingGlobalTooltipPos() const;

    // ===============================
    // Mémoire topologique absolue
    // ===============================
    std::int64_t m_lastSnapXAbs = 0;
    bool         m_hasLastSnapXAbs = false;

    std::int64_t m_lastPointXAbs = 0;
    bool         m_hasLastPointXAbs = false;

    // --------------------------------------------------------
    // Transform UNIQUE
    // --------------------------------------------------------
    void updateViewTransform();
    void applyHome();
    void panToHome();

    // --------------------------------------------------------
    // Overlays (visuels)
    // --------------------------------------------------------
    void drawVoidOverlay(QPainter* p);
    void drawBlueRuleOverlay(QPainter* p);
    void drawJonctionOverlay(QPainter* p);

    // --------------------------------------------------------
    // Utils
    // --------------------------------------------------------
    static int normalizeMod(int v, int m);
    static double normalizeModD(double v, double m);

private:
    WorkspaceScene*        m_scene = nullptr;
    Ui::Animate*           m_anim  = nullptr;
    Model::WorkspaceModel* m_model = nullptr;

    int m_ribbonOffsetMM = 0;
};


// ============================================================
// End Of File
// ============================================================
