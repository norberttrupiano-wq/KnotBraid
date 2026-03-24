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
// File        : src/domain/TopologyStore.cpp
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#include "TopologyStore.h"

#include "../core/CylinderTopology.h"

#include <QLineF>

#include <algorithm>
#include <cmath>
#include <map>

namespace
{
static QColor defaultRopeColor(std::uint8_t ropeId)
{
    static const QColor kPalette[Domain::MaxRopes] = {
        QColor(40, 120, 255),
        QColor(230, 70, 70),
        QColor(60, 170, 95),
        QColor(240, 170, 40),
        QColor(150, 90, 220)
    };

    const std::size_t idx = static_cast<std::size_t>(ropeId) % static_cast<std::size_t>(Domain::MaxRopes);
    return kPalette[idx];
}

static bool sharesEndpoint(const Domain::SegmentRef& a, const Domain::SegmentRef& b)
{
    if (a.ropeId != b.ropeId)
        return false;

    if (a.segIndex == b.segIndex)
        return true;

    return std::abs(a.segIndex - b.segIndex) == 1;
}

static double paramOnSegment(const QPointF& a, const QPointF& b, const QPointF& p)
{
    const double dx = b.x() - a.x();
    const double dy = b.y() - a.y();

    if (std::fabs(dx) >= std::fabs(dy))
    {
        if (std::fabs(dx) <= 1e-12)
            return 0.0;
        return (p.x() - a.x()) / dx;
    }

    if (std::fabs(dy) <= 1e-12)
        return 0.0;
    return (p.y() - a.y()) / dy;
}

static bool isEndpointIntersection(const QLineF& l1, const QLineF& l2, const QPointF& inter)
{
    const double t1 = paramOnSegment(l1.p1(), l1.p2(), inter);
    const double t2 = paramOnSegment(l2.p1(), l2.p2(), inter);
    const double eps = 1e-6;

    const bool end1 = (t1 <= eps) || (t1 >= 1.0 - eps);
    const bool end2 = (t2 <= eps) || (t2 >= 1.0 - eps);
    return end1 || end2;
}
} // namespace

namespace Domain
{
TopologyStore::TopologyStore(std::int32_t ribbonLengthMM)
{
    m_snapshot.ribbonLengthMM = (ribbonLengthMM > 0) ? ribbonLengthMM : 280;
    m_snapshot.ribbonOffsetMM = 0;
    m_snapshot.activeRopeId = 0;
    ensureInitialized();
    rebuildDerivedGeometry();
}

const TopologySnapshot& TopologyStore::snapshot() const
{
    return m_snapshot;
}

void TopologyStore::ensureInitialized()
{
    if (m_snapshot.ropes.size() == static_cast<std::size_t>(MaxRopes))
        return;

    m_snapshot.ropes.clear();
    m_snapshot.ropes.reserve(MaxRopes);

    for (std::uint8_t i = 0; i < MaxRopes; ++i)
    {
        RopeState rope;
        rope.ropeId = i;
        rope.color = defaultRopeColor(i);
        m_snapshot.ropes.push_back(rope);
    }
}

void TopologyStore::bumpGeneration()
{
    ++m_snapshot.generation;
}

void TopologyStore::clear()
{
    ensureInitialized();
    for (auto& rope : m_snapshot.ropes)
        rope.points.clear();

    m_snapshot.segments.clear();
    m_snapshot.crossings.clear();
    m_snapshot.activeRopeId = 0;
    m_snapshot.ribbonLengthMM = 280;
    m_snapshot.ribbonOffsetMM = 0;
    bumpGeneration();
}

void TopologyStore::clearPointsOnly()
{
    ensureInitialized();
    for (auto& rope : m_snapshot.ropes)
        rope.points.clear();

    m_snapshot.segments.clear();
    m_snapshot.crossings.clear();
    bumpGeneration();
}

void TopologyStore::setRibbonLengthMM(std::int32_t length)
{
    if (length <= 0)
        return;

    m_snapshot.ribbonLengthMM = length;
    rebuildDerivedGeometry();
}

void TopologyStore::setRibbonOffsetMM(std::int32_t offset)
{
    const int L = m_snapshot.ribbonLengthMM;
    if (L <= 0)
        return;

    int v = offset % L;
    if (v < 0) v += L;
    m_snapshot.ribbonOffsetMM = v;
    bumpGeneration();
}

void TopologyStore::setActiveRopeId(RopeId ropeId)
{
    ensureInitialized();

    if (ropeId >= MaxRopes)
        ropeId = 0;

    m_snapshot.activeRopeId = ropeId;
    bumpGeneration();
}

RopeId TopologyStore::activeRopeId() const
{
    return m_snapshot.activeRopeId;
}

void TopologyStore::setRopeColor(RopeId ropeId, const QColor& color)
{
    ensureInitialized();

    if (ropeId >= MaxRopes || !color.isValid())
        return;

    m_snapshot.ropes[static_cast<std::size_t>(ropeId)].color = color;
    bumpGeneration();
}

QColor TopologyStore::ropeColor(RopeId ropeId) const
{
    if (m_snapshot.ropes.empty())
        return QColor(40, 120, 255);

    if (ropeId >= MaxRopes || static_cast<std::size_t>(ropeId) >= m_snapshot.ropes.size())
        return m_snapshot.ropes.front().color;

    return m_snapshot.ropes[static_cast<std::size_t>(ropeId)].color;
}
void TopologyStore::appendLogicalPoint(const QPointF& logicalPoint)
{
    appendLogicalPointForRope(m_snapshot.activeRopeId, logicalPoint);
}

void TopologyStore::appendLogicalPointForRope(RopeId ropeId, const QPointF& logicalPoint)
{
    ensureInitialized();

    if (ropeId >= MaxRopes)
        ropeId = 0;

    const std::int32_t L = m_snapshot.ribbonLengthMM;
    const std::int32_t xMod = Core::CylinderTopology::wrapMod(static_cast<std::int64_t>(std::llround(logicalPoint.x())), L);
    const std::int32_t y = static_cast<std::int32_t>(std::llround(logicalPoint.y()));

    auto& rope = m_snapshot.ropes[static_cast<std::size_t>(ropeId)];

    std::int64_t xAbs = xMod;
    if (!rope.points.empty())
        xAbs = Core::CylinderTopology::unwrapNearest(xMod, rope.points.back().xAbs, L);

    rope.points.push_back(TopoPoint{xAbs, y});

    rebuildDerivedGeometry();
}

bool TopologyStore::popLastPointFromRope(RopeId ropeId)
{
    ensureInitialized();

    if (ropeId >= MaxRopes)
        return false;

    auto& rope = m_snapshot.ropes[static_cast<std::size_t>(ropeId)];
    if (rope.points.empty())
        return false;

    rope.points.pop_back();
    rebuildDerivedGeometry();
    return true;
}

bool TopologyStore::truncateRopeToPointCount(RopeId ropeId, std::size_t pointCount)
{
    ensureInitialized();

    if (ropeId >= MaxRopes)
        return false;

    auto& rope = m_snapshot.ropes[static_cast<std::size_t>(ropeId)];
    if (pointCount > rope.points.size())
        return false;

    if (pointCount == rope.points.size())
        return true;

    rope.points.resize(pointCount);
    rebuildDerivedGeometry();
    return true;
}

void TopologyStore::appendAbsPointToRope(RopeId ropeId, std::int64_t xAbs, std::int32_t y)
{
    ensureInitialized();

    if (ropeId >= MaxRopes)
        ropeId = 0;

    auto& rope = m_snapshot.ropes[static_cast<std::size_t>(ropeId)];
    rope.points.push_back(TopoPoint{xAbs, y});
}

void TopologyStore::rebuildDerivedGeometry()
{
    ensureInitialized();

    std::map<CrossingKey, bool> overByKey;
    for (const auto& c : m_snapshot.crossings)
        overByKey[c.key] = c.s2OverS1;

    m_snapshot.segments.clear();
    m_snapshot.crossings.clear();

    for (const auto& rope : m_snapshot.ropes)
    {
        if (rope.points.size() < 2)
            continue;

        for (std::size_t i = 1; i < rope.points.size(); ++i)
        {
            TopoSegment s;
            s.ref.ropeId = rope.ropeId;
            s.ref.segIndex = static_cast<std::int32_t>(i - 1);
            s.a = rope.points[i - 1];
            s.b = rope.points[i];
            m_snapshot.segments.push_back(s);
        }
    }

    const int L = m_snapshot.ribbonLengthMM;
    if (L <= 0)
    {
        bumpGeneration();
        return;
    }

    int nextCrossingId = 0;

    for (std::size_t i = 0; i < m_snapshot.segments.size(); ++i)
    {
        const auto& si = m_snapshot.segments[i];
        const double iMin = static_cast<double>(std::min(si.a.xAbs, si.b.xAbs));
        const double iMax = static_cast<double>(std::max(si.a.xAbs, si.b.xAbs));
        const double iMid = 0.5 * (static_cast<double>(si.a.xAbs) + static_cast<double>(si.b.xAbs));

        for (std::size_t j = i + 1; j < m_snapshot.segments.size(); ++j)
        {
            const auto& sj = m_snapshot.segments[j];

            if (sharesEndpoint(si.ref, sj.ref))
                continue;

            const double jMin = static_cast<double>(std::min(sj.a.xAbs, sj.b.xAbs));
            const double jMax = static_cast<double>(std::max(sj.a.xAbs, sj.b.xAbs));

            const int kMin = static_cast<int>(std::floor((iMin - jMax) / static_cast<double>(L))) - 1;
            const int kMax = static_cast<int>(std::ceil((iMax - jMin) / static_cast<double>(L))) + 1;

            bool has = false;
            QPointF bestInter;
            std::int64_t bestXAbs = 0;
            double bestScore = 1e100;

            for (int k = kMin; k <= kMax; ++k)
            {
                const double shift = static_cast<double>(k) * static_cast<double>(L);

                const QLineF li(
                    QPointF(static_cast<double>(si.a.xAbs), static_cast<double>(si.a.y)),
                    QPointF(static_cast<double>(si.b.xAbs), static_cast<double>(si.b.y))
                );

                const QLineF lj(
                    QPointF(static_cast<double>(sj.a.xAbs) + shift, static_cast<double>(sj.a.y)),
                    QPointF(static_cast<double>(sj.b.xAbs) + shift, static_cast<double>(sj.b.y))
                );

                QPointF inter;
                if (li.intersects(lj, &inter) != QLineF::BoundedIntersection)
                    continue;

                if (isEndpointIntersection(li, lj, inter))
                    continue;

                const double score = std::fabs(inter.x() - iMid);
                if (score < bestScore)
                {
                    bestScore = score;
                    bestInter = inter;
                    bestXAbs = static_cast<std::int64_t>(std::llround(inter.x()));
                    has = true;
                }
            }

            if (!has)
                continue;

            TopoCrossing c;
            c.id = nextCrossingId++;
            c.s1 = si.ref;
            c.s2 = sj.ref;
            c.xAbs = bestXAbs;
            c.y = static_cast<std::int32_t>(std::llround(bestInter.y()));
            c.turn = Core::CylinderTopology::turnOf(bestXAbs, L);

            if (si.ref < sj.ref)
            {
                c.key.sMin = si.ref;
                c.key.sMax = sj.ref;
            }
            else
            {
                c.key.sMin = sj.ref;
                c.key.sMax = si.ref;
            }
            c.key.turn = c.turn;

            auto it = overByKey.find(c.key);
            c.s2OverS1 = (it != overByKey.end()) ? it->second : true;

            m_snapshot.crossings.push_back(c);
        }
    }

    bumpGeneration();
}

bool TopologyStore::setCrossingOver(const CrossingKey& key, bool s2OverS1)
{
    ensureInitialized();

    for (auto& crossing : m_snapshot.crossings)
    {
        if (!(crossing.key < key) && !(key < crossing.key))
        {
            if (crossing.s2OverS1 == s2OverS1)
                return true;

            crossing.s2OverS1 = s2OverS1;
            bumpGeneration();
            return true;
        }
    }

    return false;
}
} // namespace Domain

// ============================================================
// End Of File
// ============================================================
