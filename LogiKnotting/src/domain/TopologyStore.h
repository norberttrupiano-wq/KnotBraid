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
// File        : src/domain/TopologyStore.h
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#pragma once

#include "TopologyTypes.h"

#include <QPointF>

namespace Domain
{
class TopologyStore
{
public:
    explicit TopologyStore(std::int32_t ribbonLengthMM = 280);

    const TopologySnapshot& snapshot() const;

    void clear();
    void clearPointsOnly();

    void setRibbonLengthMM(std::int32_t length);
    void setRibbonOffsetMM(std::int32_t offset);

    void setActiveRopeId(RopeId ropeId);
    RopeId activeRopeId() const;
    void setRopeColor(RopeId ropeId, const QColor& color);
    QColor ropeColor(RopeId ropeId) const;

    void appendLogicalPoint(const QPointF& logicalPoint);
    void appendLogicalPointForRope(RopeId ropeId, const QPointF& logicalPoint);
    bool popLastPointFromRope(RopeId ropeId);

    void appendAbsPointToRope(RopeId ropeId, std::int64_t xAbs, std::int32_t y);
    void rebuildDerivedGeometry();
    bool setCrossingOver(const CrossingKey& key, bool s2OverS1);

private:
    void ensureInitialized();
    void bumpGeneration();

private:
    TopologySnapshot m_snapshot;
};
} // namespace Domain

// ============================================================
// End Of File
// ============================================================
