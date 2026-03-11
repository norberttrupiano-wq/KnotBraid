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
// File        : src/model/WorkspaceModel.h
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#pragma once

#include <QColor>
#include <QLineF>
#include <QPointF>
#include <QString>

#include <cstdint>
#include <stack>
#include <vector>

#include "domain/TopologyStore.h"

namespace Model
{
// ------------------------------------------------------------
// Orientation
// ------------------------------------------------------------
enum class Orientation : uint8_t
{
    Deg0,    // 0Ã‚Â° / 180Ã‚Â°
    Deg45,   // 45Ã‚Â° / 225Ã‚Â°
    Deg90,   // 90Ã‚Â° / 270Ã‚Â°
    Deg135   // 135Ã‚Â° / 315Ã‚Â°
};

// ------------------------------------------------------------
// Action
// ------------------------------------------------------------
struct Action
{
    enum class Type
    {
        AddPoint
    };

    Type    type;
    QPointF positionMM;
    int     ropeId = 0; // MIGRATION-PARALLEL: rope active au moment de l'ajout
};

// ------------------------------------------------------------
// Crossing
// ------------------------------------------------------------
struct Crossing
{
    QPointF positionMM;

    int     segmentAIndex = -1;
    int     segmentBIndex = -1;

    bool    newSegmentOver = true;

    int     tour = 0;   // V2 Ã¢â‚¬â€ numÃƒÂ©ro de tour (franchissement jonction)
};

struct FileHistoryEntry
{
    QString action;
    QString authorId;
    QString atUtcIso;
    std::int64_t workSeconds = 0;
};

// ------------------------------------------------------------
// WorkspaceModel
// ------------------------------------------------------------
class WorkspaceModel
{
public:
    WorkspaceModel();
    const std::vector<Orientation>& segmentOrientations() const;
    Orientation segmentOrientation(size_t index) const;

    void addPointMM(const QPointF& posMM);
    void addPoint(const QPointF& posMM);

    bool canUndo() const;
    bool canRedo() const;
    void undo();
    void redo();
    void clear();
    bool wrapIsDone() const;   // un vrap as-t-il eu lieu ?
    int bightCount() const;     // nombre de franchissements de jonction
    bool canResizeRibbon(int deltaMM) const;
    void resizeRibbonMM(int deltaMM);

    const std::vector<QPointF>&  points() const;
    const std::vector<QLineF>&   segments() const;
    const std::vector<Crossing>& crossings() const;
    void invertCrossing(std::size_t index);

    int ribbonLengthMM() const;
    int ribbonOffsetMM() const;

    bool canValidateAsLocked() const;

    void rotateRibbonMM(int deltaMM);
    void setRibbonLengthMM(int value);
    void setRibbonOffsetMM(int value);

    bool saveToFile(const QString& filePath) const;
    bool loadFromFile(const QString& filePath);

    std::int64_t designTimeSeconds() const;
    void resetDesignTime();

    void initializeAuditForNewDocument(const QString& creatorAuthorId);
    void appendAuditOnSave(const QString& authorId, std::int64_t workSeconds);
    QString filePropertiesText() const;

    // MIGRATION-PARALLEL: API multi-cordes (n'affecte pas le rendu legacy)
    void setActiveRopeId(int ropeId);
    int activeRopeId() const;
    QColor activeRopeColor() const;
    void setRopeColor(int ropeId, const QColor& color);
    QColor ropeColor(int ropeId) const;
    const Domain::TopologySnapshot& topologySnapshot() const;

private:
    void apply(const Action& action);
    void revert(const Action& action);

    void rebuildSegments();
    void rebuildCrossings();

    // Recalcule m_pointsXAbs de faÃƒÂ§on dÃƒÂ©terministe ÃƒÂ  partir de m_points
    // (utilisÃƒÂ© au Load / Redo / changements de rÃƒÂ©fÃƒÂ©rentiel).
    void rebuildPointsXAbs();

    void syncTopologyStoreFromLegacy();

    Orientation computeOrientation(const QLineF& seg) const;

    void startDesignTimeIfNeeded();
    void updateDesignTimeAccumulated();
    void appendAuditEntry(const QString& action,
                         const QString& authorId,
                         const QString& atUtcIso,
                         std::int64_t workSeconds);

private:
    std::vector<QPointF>        m_points;
    std::vector<std::int64_t>   m_pointsXAbs;
    std::vector<QLineF>         m_segments;
    std::vector<Crossing>       m_crossings;

    std::stack<Action>          m_undoStack;
    std::stack<Action>          m_redoStack;
    std::vector<Orientation>    m_segmentOrientations;

    int m_ribbonLengthMM = 280;
    int m_ribbonOffsetMM = 0;

    std::int64_t m_designTimeSeconds = 0;
    bool         m_designTimeRunning = false;
    std::int64_t m_designTimeStartMS = 0;
    std::int64_t m_lastDesignActivityMS = 0;
    static constexpr std::int64_t kDesignIdleTimeoutMS = 15000;

    // MIGRATION-PARALLEL: nouveau noyau en parallÃƒÂ¨le du legacy
    QString m_creatorAuthorId;
    QString m_createdAtUtcIso;
    QString m_lastModifiedByAuthorId;
    QString m_lastModifiedAtUtcIso;
    std::int64_t m_totalWorkSeconds = 0;
    std::vector<FileHistoryEntry> m_fileHistory;

    Domain::TopologyStore m_topologyStore;
};

}

// ============================================================
// End Of File
// ============================================================


