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
#include <QJsonObject>
#include <QPointF>
#include <QLineF>
#include <QRectF>
#include <cstdint>
#include <vector>

#include "../domain/TopologyTypes.h"

namespace Ui { class Animate; }

class WorkspaceScene;
namespace Model { class WorkspaceModel; }

class WorkspaceView : public QGraphicsView
{
    Q_OBJECT

    // ------------------------------------------------------------
    // Modes Esquisse / TraÃƒÆ’Ã‚Â§age
    // ------------------------------------------------------------
public slots:
    void enterSketchMode();
    void breakSketch();
    void enterTracingMode();
    void enterCrossingEditMode();
    void zoomInView();
    void zoomOutView();
    void zoomResetView();
    void triggerUndo();
    void triggerRedo();
    void rotateSelectionRight90();
    void rotateSelectionLeft90();
    void flipSelectionVertically();
    void flipSelectionHorizontally();

private:
    bool m_sketchMode = false;

    // --------------------------------------------------------
    // Esquisse = clics snapÃƒÆ’Ã‚Â©s (pas de "crayon libre")
    // - 1er clic : pose lÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ancre
    // - clics suivants : segment (ancre -> point) puis nouvelle ancre
    // - Space : sectionner (perd lÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ancre, on repart ailleurs)
    // --------------------------------------------------------
    bool                    m_sketchHasAnchor = false;
    QPointF                 m_sketchAnchorMM;
    bool                    m_hasPreviousSketchAnchor = false;
    QPointF                 m_previousSketchAnchorMM;
    std::vector<std::vector<QPointF>> m_sketchGuidePaths;
    std::vector<QLineF>     m_sketchSegments;
    std::vector<QPointF>    m_sketchPoints;   // optionnel (trace/diagnostic)

    struct SketchSnapshot
    {
        std::vector<QLineF> segments;
        std::vector<QPointF> points;
        bool hasAnchor = false;
        QPointF anchor;
        bool hasPreviousAnchor = false;
        QPointF previousAnchor;
        bool sketchBreak = false;
    };

    std::vector<SketchSnapshot> m_sketchUndoStack;
    std::vector<SketchSnapshot> m_sketchRedoStack;

    bool m_sketchSelecting = false;
    bool m_hasSketchSelectionRect = false;
    QPointF m_sketchSelectionStartMM;
    QPointF m_sketchSelectionEndMM;
    std::vector<int> m_sketchSelectedSegmentIndices;

    bool m_sketchEndpointDragPending = false;
    bool m_sketchEndpointDragging = false;
    bool m_sketchEndpointUndoPrimed = false;
    int m_sketchEndpointSegmentIndex = -1;
    bool m_sketchEndpointMovesP1 = false;
    QPointF m_sketchEndpointPressMM;
    QPointF m_sketchEndpointOriginalPointMM;
    QPointF m_sketchEndpointFixedPointMM;
    QPointF m_sketchEndpointFixedVisibleMM;
    QPointF m_sketchEndpointLastDisplayMM;

    bool m_sketchTransforming = false;
    bool m_sketchTransformCopy = false;
    bool m_sketchTransformUndoPrimed = false;
    bool m_sketchTransformHadRect = false;
    QPointF m_sketchTransformStartMM;
    QPointF m_sketchTransformLastMM;
    QPointF m_sketchTransformRectStartMM;
    QPointF m_sketchTransformRectEndMM;
    std::vector<int> m_sketchTransformIndices;
    std::vector<QLineF> m_sketchTransformSourceSegments;
    std::vector<QLineF> m_sketchTransformBaseSegments;

    bool m_hasSketchClipboard = false;
    QPointF m_sketchClipboardOriginMM;
    std::vector<QLineF> m_sketchClipboardSegments;

    bool m_hasLastSketchCursor = false;
    QPointF m_lastSketchCursorMM;
    bool m_hasPreviewDisplayReference = false;
    QPointF m_previewDisplayReferenceMM;

public:
    explicit WorkspaceView(QWidget* parent = nullptr);
    void setModel(Model::WorkspaceModel* model);
    void setReadOnlyValidated(bool enabled);
    bool isReadOnlyValidated() const { return m_readOnlyValidated; }
    bool isSketchMode() const { return m_sketchMode; }
    void copyModelGeometryToSketch();
    void clearSketchOverlay();
    QJsonObject exportSketchOverlayState() const;
    void importSketchOverlayState(const QJsonObject& sketchOverlayState);

    // --------------------------------------------------------
    // Animation
    // --------------------------------------------------------
    void startAnimationSimple();
    void startAnimationSerpent();
    void stopAnimation();
    bool isAnimating() const;

    // Menu "Play Animation" (LK-STRICT) :
    // - lance Serpent par dÃƒÆ’Ã‚Â©faut (pÃƒÆ’Ã‚Â©dagogique + joli)
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
    void interactionModeChanged();

protected:
    void paintEvent(QPaintEvent* e) override;

    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

private:
    QPointF interpretPixelAsWorld(const QPoint& vp) const;

    SketchSnapshot captureSketchSnapshot() const;
    void restoreSketchSnapshot(const SketchSnapshot& snapshot);
    void pushSketchUndoSnapshot();
    bool undoSketch();
    bool redoSketch();
    void clearSketchHistory();

    void clearSketchSelection();
    QRectF currentSketchSelectionRectMM() const;
    void updateSketchSelectionFromRect();
    bool collectSelectedSketchSegments(std::vector<int>* indices, QRectF* boundsMM) const;
    void finalizeSketchSelectionTransform(const std::vector<int>& indices);
    bool copySketchSelectionToClipboard();
    bool pasteSketchClipboardAt(const QPointF& targetAbsMM);
    bool deleteSketchSelection();
    bool handleSketchLeftClick(const QPointF& scenePos, Qt::KeyboardModifiers modifiers);
    bool findResizableSketchEndpoint(const QPointF& displayAbsMM,
                                     int* segmentIndex,
                                     bool* endpointIsP1,
                                     QPointF* endpointAbsMM,
                                     QPointF* fixedAbsMM,
                                     QPointF* fixedVisibleMM,
                                     QPointF* endpointVisibleMM) const;

    bool hasSketchSelectionAt(const QPointF& absMM) const;
    bool beginSketchTransformDrag(const QPointF& startAbsMM, bool copyMode);
    void updateSketchTransformDrag(const QPointF& currentAbsMM);
    void finishSketchTransformDrag(const QPointF& endAbsMM);
    void cancelSketchTransformDrag();
    void beginSketchEndpointDrag();
    void updateSketchEndpointDrag(const QPointF& currentDisplayAbsMM, int snapStep);
    void finishSketchEndpointDrag(const QPointF& endDisplayAbsMM, int snapStep);
    void cancelSketchEndpointDrag();
    void clearPendingSketchEndpointDrag();
    void replaceSketchStoredPoint(const QPointF& oldPoint, const QPointF& newPoint);

    void rebuildSketchPointsFromSegments();
    void rebuildSketchOverlayFromGuidePaths();
    bool consumeGuidePathByTrace(const QLineF& tracedAbsSegment);
    bool eraseSketchCoveredBySegment(const QLineF& tracedAbsSegment);
    bool findTracingSegmentNear(const QPointF& absMM, Domain::SegmentRef* outRef) const;
    void clearTracingSegmentSelection();
    bool deleteTracingSegmentSelection();
    bool resolveSketchPointFromDisplay(const QPointF& clickedDisplayAbsMM,
                                      QPointF* resolvedAbsMM) const;
    bool resolveConnectedSketchEndpoint(const QPointF& currentAbsMM,
                                       const QPointF& clickedDisplayAbsMM,
                                       QPointF* resolvedAbsMM,
                                       const QPointF* previousAbsMM = nullptr) const;

    QPointF m_previewStart;
    QPointF m_previewEnd;
    bool    m_previewValid = false;
    bool    m_hasPreview   = false;
 //   std::vector<QPointF> m_sketchPoints;
 //   std::vector<QLineF>  m_sketchSegments;
    bool m_sketchBreak = false;
//    bool m_sketchMode  = false;
    //========================================================
    // Mode Crossings (LK-STRICT)
    //========================================================
    bool m_crossingEditMode = false;
    bool m_readOnlyValidated = false;
    int  m_activeCrossingIndex = -1;

    void exitCrossingEditMode();
    void setActiveCrossingIndex(int idx, bool recenter);
    void centerOnActiveCrossingX();
    void warpCursorToActiveCrossing();
    QPoint activeCrossingGlobalTooltipPos() const;
    void showTransientStatusMessage(const QString& text, int timeoutMs = 3000) const;

    // ===============================
    // MÃƒÆ’Ã‚Â©moire topologique absolue
    // ===============================
    std::int64_t m_lastSnapXAbs = 0;
    bool         m_hasLastSnapXAbs = false;

    std::int64_t m_lastPointXAbs = 0;
    bool         m_hasLastPointXAbs = false;
    Domain::SegmentRef m_tracingSelectedSegment;

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
    double m_zoomFactor = 1.0;
};


// ============================================================
// End Of File
// ============================================================

