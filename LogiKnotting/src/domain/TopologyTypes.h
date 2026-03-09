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
// File        : src/domain/TopologyTypes.h
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#pragma once

#include <QColor>

#include <cstdint>
#include <vector>

namespace Domain
{
static constexpr std::uint8_t MaxRopes = 5;

using RopeId = std::uint8_t;

struct SegmentRef
{
    RopeId ropeId = 0;
    std::int32_t segIndex = -1;

    bool operator<(const SegmentRef& other) const
    {
        if (ropeId != other.ropeId) return ropeId < other.ropeId;
        return segIndex < other.segIndex;
    }

    bool operator==(const SegmentRef& other) const
    {
        return ropeId == other.ropeId && segIndex == other.segIndex;
    }
};

struct TopoPoint
{
    std::int64_t xAbs = 0;
    std::int32_t y = 0;
};

struct RopeState
{
    RopeId ropeId = 0;
    QColor color;
    std::vector<TopoPoint> points;
};

struct TopoSegment
{
    SegmentRef ref;
    TopoPoint a;
    TopoPoint b;
};

struct CrossingKey
{
    SegmentRef sMin;
    SegmentRef sMax;
    std::int32_t turn = 0;

    bool operator<(const CrossingKey& other) const
    {
        if (sMin < other.sMin) return true;
        if (other.sMin < sMin) return false;
        if (sMax < other.sMax) return true;
        if (other.sMax < sMax) return false;
        return turn < other.turn;
    }
};

struct TopoCrossing
{
    std::int32_t id = -1;
    SegmentRef s1;
    SegmentRef s2;
    std::int64_t xAbs = 0;
    std::int32_t y = 0;
    std::int32_t turn = 0;
    bool s2OverS1 = true;
    CrossingKey key;
};

struct TopologySnapshot
{
    std::uint64_t generation = 0;
    std::int32_t ribbonLengthMM = 280;
    std::int32_t ribbonOffsetMM = 0;
    RopeId activeRopeId = 0;
    std::vector<RopeState> ropes;
    std::vector<TopoSegment> segments;
    std::vector<TopoCrossing> crossings;
};
} // namespace Domain

// ============================================================
// End Of File
// ============================================================
