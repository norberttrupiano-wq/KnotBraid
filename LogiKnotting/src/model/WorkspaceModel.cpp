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
// File        : src/model/WorkspaceModel.cpp
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#include "workspacemodel.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QRandomGenerator>
#include <QStringList>

#include <cassert>
#include <algorithm>
#include <cmath>
#include <map>

namespace Model
{

// ------------------------------------------------------------
// Helpers i64
// ------------------------------------------------------------
static std::int64_t normalizeModI64(std::int64_t v, std::int64_t m)
{
    if (m <= 0) return 0;
    const std::int64_t r = v % m;
    return (r < 0) ? (r + m) : r;
}

static int floorDivI64(std::int64_t a, std::int64_t b)
{
    // division entiÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¨re "mathÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â©matique" (floor), b > 0
    if (b <= 0) return 0;
    if (a >= 0) return static_cast<int>(a / b);
    return static_cast<int>(- ((-a + b - 1) / b));
}

static Domain::CrossingKey legacyCrossingKey(const Model::Crossing& crossing)
{
    Domain::CrossingKey key;
    key.sMin.ropeId = 0;
    key.sMax.ropeId = 0;
    key.sMin.segIndex = std::min(crossing.segmentAIndex, crossing.segmentBIndex);
    key.sMax.segIndex = std::max(crossing.segmentAIndex, crossing.segmentBIndex);
    key.turn = crossing.tour;
    return key;
}

// ------------------------------------------------------------

static QString authorIdFromEmail(const QString& email)
{
    const QByteArray digest = QCryptographicHash::hash(email.trimmed().toLower().toUtf8(),
                                                       QCryptographicHash::Sha256)
                                  .toHex()
                                  .toUpper();
    return QString::fromLatin1(digest.left(12));
}

static QString canonicalAuthorId(const QString& raw)
{
    const QString s = raw.trimmed();
    if (s.isEmpty())
        return QString();

    if (s.contains(QLatin1Char('@')))
        return authorIdFromEmail(s);

    QString out;
    out.reserve(s.size());
    for (const QChar ch : s.toUpper())
    {
        if (ch.isDigit() || (ch >= QLatin1Char('A') && ch <= QLatin1Char('F')))
            out.push_back(ch);
    }

    if (out.size() > 12)
        out = out.left(12);

    return out;
}

static constexpr const char* kIntegrityAlgorithm = "LK-HMAC-SHA256-V1";
static constexpr const char* kIntegritySecret =
    "LogiKnotting::WorkspaceModel::Integrity::2026::A3B19F4721CDE98A";

static QString makeIntegrityNonce()
{
    QByteArray nonce;
    nonce.resize(16);

    for (int i = 0; i < nonce.size(); ++i)
        nonce[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));

    return QString::fromLatin1(nonce.toHex().toUpper());
}

static QString computeIntegritySignature(const QJsonObject& payloadRoot, const QString& nonce)
{
    const QByteArray payload = QJsonDocument(payloadRoot).toJson(QJsonDocument::Compact);

    QByteArray blob;
    blob.reserve(payload.size() + nonce.size() + 128);
    blob.append(kIntegrityAlgorithm);
    blob.append('|');
    blob.append(nonce.toUtf8());
    blob.append('|');
    blob.append(kIntegritySecret);
    blob.append('|');
    blob.append(payload);

    const QByteArray digest = QCryptographicHash::hash(blob, QCryptographicHash::Sha256)
                                  .toHex()
                                  .toUpper();
    return QString::fromLatin1(digest);
}

static bool extractAndVerifyIntegrity(const QJsonObject& rootWithIntegrity, QJsonObject& payloadOut)
{
    QJsonObject payload = rootWithIntegrity;
    const QJsonValue integrityValue = payload.take(QStringLiteral("integrity"));
    if (!integrityValue.isObject())
        return false;

    const QJsonObject integrity = integrityValue.toObject();
    const QString alg = integrity.value(QStringLiteral("alg")).toString().trimmed();
    const QString nonce = integrity.value(QStringLiteral("nonce")).toString().trimmed();
    const QString sig = integrity.value(QStringLiteral("sig")).toString().trimmed().toUpper();

    if (alg != QString::fromLatin1(kIntegrityAlgorithm))
        return false;
    if (nonce.isEmpty() || sig.isEmpty())
        return false;

    const QString expectedSig = computeIntegritySignature(payload, nonce);
    if (expectedSig != sig)
        return false;

    payloadOut = payload;
    return true;
}

// Intersection helper
// ------------------------------------------------------------
static bool segmentsIntersect(const QLineF& a,
                              const QLineF& b,
                              QPointF* intersection = nullptr)
{
    QPointF pt;
    if (a.intersects(b, &pt) == QLineF::BoundedIntersection)
    {
        if (intersection)
            *intersection = pt;

        return true;
    }

    return false;
}

// ------------------------------------------------------------
// Getters
// ------------------------------------------------------------
const std::vector<QPointF>& WorkspaceModel::points() const
{
    return m_points;
}

const std::vector<QLineF>& WorkspaceModel::segments() const
{
    return m_segments;
}

Model::Orientation
WorkspaceModel::segmentOrientation(std::size_t index) const
{
    return m_segmentOrientations.at(index);
}

const std::vector<Model::Orientation>&
WorkspaceModel::segmentOrientations() const
{
    return m_segmentOrientations;
}

const std::vector<Crossing>& WorkspaceModel::crossings() const
{
    return m_crossings;
}

void WorkspaceModel::setActiveRopeId(int ropeId)
{
    m_topologyStore.setActiveRopeId(static_cast<Domain::RopeId>(ropeId));
    syncLegacyFromTopologyRope(static_cast<Domain::RopeId>(ropeId));
}

int WorkspaceModel::activeRopeId() const
{
    return static_cast<int>(m_topologyStore.activeRopeId());
}

QColor WorkspaceModel::activeRopeColor() const
{
    const auto& s = m_topologyStore.snapshot();
    if (s.ropes.empty())
        return QColor(40, 120, 255);

    const std::size_t idx = static_cast<std::size_t>(s.activeRopeId);
    if (idx >= s.ropes.size())
        return s.ropes.front().color;

    return s.ropes[idx].color;
}

void WorkspaceModel::setRopeColor(int ropeId, const QColor& color)
{
    if (ropeId < 0 || ropeId >= static_cast<int>(Domain::MaxRopes) || !color.isValid())
        return;

    m_topologyStore.setRopeColor(static_cast<Domain::RopeId>(ropeId), color);
}

QColor WorkspaceModel::ropeColor(int ropeId) const
{
    if (ropeId < 0 || ropeId >= static_cast<int>(Domain::MaxRopes))
        return QColor(40, 120, 255);

    return m_topologyStore.ropeColor(static_cast<Domain::RopeId>(ropeId));
}
const Domain::TopologySnapshot& WorkspaceModel::topologySnapshot() const
{
    return m_topologyStore.snapshot();
}

bool WorkspaceModel::truncateRopeAfterSegment(const Domain::SegmentRef& ref)
{
    const auto& topo = m_topologyStore.snapshot();
    const std::size_t ropeIndex = static_cast<std::size_t>(ref.ropeId);
    if (ropeIndex >= topo.ropes.size() || ref.segIndex < 0)
        return false;

    const auto& rope = topo.ropes[ropeIndex];
    const std::size_t keepPointCount = static_cast<std::size_t>(ref.segIndex) + 1;
    if (keepPointCount >= rope.points.size())
        return false;

    startDesignTimeIfNeeded();

    if (!m_topologyStore.truncateRopeToPointCount(ref.ropeId, keepPointCount))
        return false;

    m_topologyStore.setActiveRopeId(ref.ropeId);
    syncLegacyFromTopologyRope(ref.ropeId);

    while (!m_undoStack.empty())
        m_undoStack.pop();
    while (!m_redoStack.empty())
        m_redoStack.pop();

    return true;
}

void WorkspaceModel::setSketchOverlayState(const QJsonObject& sketchOverlayState)
{
    m_sketchOverlayState = sketchOverlayState;
}

QJsonObject WorkspaceModel::sketchOverlayState() const
{
    return m_sketchOverlayState;
}

void WorkspaceModel::clearSketchOverlayState()
{
    m_sketchOverlayState = QJsonObject();
}

bool WorkspaceModel::canValidateAsLocked() const
{
    const auto& topo = m_topologyStore.snapshot();
    const int L = topo.ribbonLengthMM;
    if (L <= 0)
        return false;

    bool hasAnyRope = false;

    for (const auto& rope : topo.ropes)
    {
        if (rope.points.empty())
            continue;

        hasAnyRope = true;

        if (rope.points.size() < 2)
            return false;

        const auto& first = rope.points.front();
        const auto& last = rope.points.back();

        const int firstX = static_cast<int>(normalizeModI64(first.xAbs, static_cast<std::int64_t>(L)));
        const int lastX = static_cast<int>(normalizeModI64(last.xAbs, static_cast<std::int64_t>(L)));

        if (firstX != lastX || first.y != last.y)
            return false;
    }

    return hasAnyRope;
}

// ------------------------------------------------------------
// Construction
// ------------------------------------------------------------
WorkspaceModel::WorkspaceModel()
{
    m_topologyStore.setRibbonLengthMM(m_ribbonLengthMM);
    m_topologyStore.setRibbonOffsetMM(m_ribbonOffsetMM);
}

// ------------------------------------------------------------
// Temps de conception
// ------------------------------------------------------------
void WorkspaceModel::startDesignTimeIfNeeded()
{
    const std::int64_t now = QDateTime::currentMSecsSinceEpoch();

    if (!m_designTimeRunning)
    {
        m_designTimeRunning = true;
        m_designTimeStartMS = now;
        m_lastDesignActivityMS = now;
        return;
    }

    const std::int64_t lastActivity = (m_lastDesignActivityMS > 0) ? m_lastDesignActivityMS : m_designTimeStartMS;
    const std::int64_t idleGapMS = now - lastActivity;

    if (idleGapMS > kDesignIdleTimeoutMS)
    {
        const std::int64_t activeEnd = lastActivity + kDesignIdleTimeoutMS;
        const std::int64_t deltaMS = activeEnd - m_designTimeStartMS;
        if (deltaMS > 0)
            m_designTimeSeconds += (deltaMS / 1000);

        m_designTimeStartMS = now;
    }
    else
    {
        const std::int64_t deltaMS = now - m_designTimeStartMS;
        if (deltaMS > 0)
            m_designTimeSeconds += (deltaMS / 1000);

        m_designTimeStartMS = now;
    }

    m_lastDesignActivityMS = now;
}

void WorkspaceModel::updateDesignTimeAccumulated()
{
    if (!m_designTimeRunning)
        return;

    const std::int64_t now = QDateTime::currentMSecsSinceEpoch();
    const std::int64_t lastActivity = (m_lastDesignActivityMS > 0) ? m_lastDesignActivityMS : now;
    const std::int64_t activeEnd = std::min(now, lastActivity + kDesignIdleTimeoutMS);
    const std::int64_t deltaMS = activeEnd - m_designTimeStartMS;

    if (deltaMS > 0)
        m_designTimeSeconds += (deltaMS / 1000);

    m_designTimeStartMS = activeEnd;
}

std::int64_t WorkspaceModel::designTimeSeconds() const
{
    if (!m_designTimeRunning)
        return m_designTimeSeconds;

    const std::int64_t now = QDateTime::currentMSecsSinceEpoch();
    std::int64_t effectiveNow = now;

    if (m_lastDesignActivityMS > 0)
        effectiveNow = std::min(now, m_lastDesignActivityMS + kDesignIdleTimeoutMS);

    const std::int64_t deltaMS = effectiveNow - m_designTimeStartMS;
    if (deltaMS <= 0)
        return m_designTimeSeconds;

    return m_designTimeSeconds + (deltaMS / 1000);
}

void WorkspaceModel::resetDesignTime()
{
    m_designTimeSeconds = 0;
    m_designTimeRunning = false;
    m_designTimeStartMS = 0;
    m_lastDesignActivityMS = 0;
}


void WorkspaceModel::appendAuditEntry(const QString& action,
                                      const QString& authorId,
                                      const QString& atUtcIso,
                                      std::int64_t workSeconds)
{
    FileHistoryEntry entry;
    entry.action = action.trimmed().toLower();
    entry.authorId = canonicalAuthorId(authorId);
    entry.atUtcIso = atUtcIso.trimmed();
    entry.workSeconds = std::max<std::int64_t>(0, workSeconds);
    m_fileHistory.push_back(entry);
}

void WorkspaceModel::initializeAuditForNewDocument(const QString& creatorAuthorId)
{
    const QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    m_creatorAuthorId = canonicalAuthorId(creatorAuthorId);
    m_createdAtUtcIso = now;
    m_lastModifiedByAuthorId.clear();
    m_lastModifiedAtUtcIso.clear();
    m_totalWorkSeconds = 0;
    m_fileHistory.clear();

    if (!m_creatorAuthorId.isEmpty())
        appendAuditEntry(QStringLiteral("created"), m_creatorAuthorId, now, 0);
}

void WorkspaceModel::appendAuditOnSave(const QString& authorId, std::int64_t workSeconds)
{
    const QString author = canonicalAuthorId(authorId);
    const QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    const std::int64_t safeWork = std::max<std::int64_t>(0, workSeconds);

    if (m_creatorAuthorId.isEmpty())
    {
        m_creatorAuthorId = author;
        m_createdAtUtcIso = now;
    }
    else if (m_createdAtUtcIso.isEmpty())
    {
        m_createdAtUtcIso = now;
    }

    m_lastModifiedByAuthorId = author;
    m_lastModifiedAtUtcIso = now;
    m_totalWorkSeconds += safeWork;

    const QString action = m_fileHistory.empty()
        ? QStringLiteral("created")
        : QStringLiteral("modified");
    appendAuditEntry(action, author, now, safeWork);
}

QString WorkspaceModel::filePropertiesText() const
{
    QStringList lines;
    lines << QStringLiteral("Created by: %1").arg(m_creatorAuthorId.isEmpty() ? QStringLiteral("(unknown)") : m_creatorAuthorId);
    lines << QStringLiteral("Created (UTC): %1").arg(m_createdAtUtcIso.isEmpty() ? QStringLiteral("(unknown)") : m_createdAtUtcIso);
    lines << QStringLiteral("Last modified by: %1").arg(m_lastModifiedByAuthorId.isEmpty() ? QStringLiteral("(none)") : m_lastModifiedByAuthorId);
    lines << QStringLiteral("Last modified (UTC): %1").arg(m_lastModifiedAtUtcIso.isEmpty() ? QStringLiteral("(none)") : m_lastModifiedAtUtcIso);
    lines << QStringLiteral("Total work time: %1 s").arg(m_totalWorkSeconds);
    lines << QStringLiteral("Design time counter: %1 s").arg(designTimeSeconds());
    lines << QString();
    lines << QStringLiteral("History:");

    if (m_fileHistory.empty())
    {
        lines << QStringLiteral("(empty)");
    }
    else
    {
        for (const FileHistoryEntry& e : m_fileHistory)
        {
            const QString verb = (e.action == QStringLiteral("created"))
                                     ? QStringLiteral("Created by")
                                     : QStringLiteral("Modified by");

            lines << QStringLiteral("%1: %2 | %3 | %4 s")
                         .arg(verb,
                              e.authorId.isEmpty() ? QStringLiteral("(unknown)") : e.authorId,
                              e.atUtcIso.isEmpty() ? QStringLiteral("(unknown)") : e.atUtcIso)
                         .arg(e.workSeconds);
        }
    }

    return lines.join(QLatin1Char('\n'));
}

// Inversion de croisements
// ------------------------------------------------------------
void WorkspaceModel::invertCrossing(std::size_t index)
{
    if (index >= m_crossings.size())
        return;

    startDesignTimeIfNeeded();

    m_crossings[index].newSegmentOver = !m_crossings[index].newSegmentOver;
}

bool WorkspaceModel::setTopologyCrossingOver(const Domain::CrossingKey& key, bool s2OverS1)
{
    if (!m_topologyStore.setCrossingOver(key, s2OverS1))
        return false;

    startDesignTimeIfNeeded();

    for (Crossing& crossing : m_crossings)
    {
        if (!(legacyCrossingKey(crossing) < key) && !(key < legacyCrossingKey(crossing)))
            crossing.newSegmentOver = s2OverS1;
    }

    return true;
}

// ------------------------------------------------------------
// Rebuild XAbs
// ------------------------------------------------------------
void WorkspaceModel::rebuildPointsXAbs()
{
    m_pointsXAbs.clear();
    m_pointsXAbs.reserve(m_points.size());

    if (m_points.empty())
        return;

    const std::int64_t L = static_cast<std::int64_t>(m_ribbonLengthMM);
    if (L <= 0)
        return;

    for (size_t i = 0; i < m_points.size(); ++i)
    {
        const std::int64_t x = static_cast<std::int64_t>(std::llround(m_points[i].x()));

        if (m_pointsXAbs.empty())
        {
            m_pointsXAbs.push_back(x);
            continue;
        }

        const std::int64_t lastXAbs = m_pointsXAbs.back();
        const std::int64_t k0 = floorDivI64(lastXAbs, L);

        std::int64_t best = x + k0 * L;
        std::int64_t bestDist = std::llabs(best - lastXAbs);

        for (int dk = -1; dk <= 1; ++dk)
        {
            const std::int64_t cand = x + (k0 + dk) * L;
            const std::int64_t dist = std::llabs(cand - lastXAbs);
            if (dist < bestDist)
            {
                bestDist = dist;
                best = cand;
            }
        }

        m_pointsXAbs.push_back(best);
    }
}

std::int64_t WorkspaceModel::resolveAbsoluteX(const Action& action) const
{
    if (action.hasAbsoluteX)
        return action.absoluteXMM;

    const std::int64_t x = static_cast<std::int64_t>(std::llround(action.positionMM.x()));
    if (m_pointsXAbs.empty())
        return x;

    const std::int64_t L = static_cast<std::int64_t>(m_ribbonLengthMM);
    if (L <= 0)
        return x;

    const std::int64_t lastXAbs = m_pointsXAbs.back();
    const std::int64_t k0 = floorDivI64(lastXAbs, L);

    std::int64_t bestXAbs = x + k0 * L;
    std::int64_t bestDist = std::llabs(bestXAbs - lastXAbs);

    for (int dk = -1; dk <= 1; ++dk)
    {
        const std::int64_t candidate = x + (k0 + dk) * L;
        const std::int64_t dist = std::llabs(candidate - lastXAbs);

        if (dist < bestDist)
        {
            bestDist = dist;
            bestXAbs = candidate;
        }
    }

    return bestXAbs;
}

void WorkspaceModel::syncTopologyStoreFromLegacy()
{
    // MIGRATION-PARALLEL: fallback si un fichier V2 ne contient pas le bloc ropes
    m_topologyStore.clearPointsOnly();
    m_topologyStore.setRibbonLengthMM(m_ribbonLengthMM);
    m_topologyStore.setRibbonOffsetMM(m_ribbonOffsetMM);
    m_topologyStore.setActiveRopeId(0);

    for (std::size_t i = 0; i < m_points.size() && i < m_pointsXAbs.size(); ++i)
    {
        const auto xAbs = m_pointsXAbs[i];
        const auto y = static_cast<std::int32_t>(std::llround(m_points[i].y()));
        m_topologyStore.appendAbsPointToRope(0, xAbs, y);
    }

    m_topologyStore.rebuildDerivedGeometry();
    m_topologyStore.setActiveRopeId(0);

    for (const Crossing& crossing : m_crossings)
    {
        if (crossing.segmentAIndex < 0 || crossing.segmentBIndex < 0)
            continue;

        m_topologyStore.setCrossingOver(legacyCrossingKey(crossing), crossing.newSegmentOver);
    }
}

void WorkspaceModel::syncLegacyFromTopologyRope(Domain::RopeId ropeId)
{
    m_points.clear();
    m_pointsXAbs.clear();
    m_segments.clear();
    m_segmentOrientations.clear();
    m_crossings.clear();

    const auto& topo = m_topologyStore.snapshot();
    const std::size_t ropeIndex = static_cast<std::size_t>(ropeId);
    if (ropeIndex >= topo.ropes.size())
        return;

    const auto& rope = topo.ropes[ropeIndex];
    m_points.reserve(rope.points.size());
    m_pointsXAbs.reserve(rope.points.size());

    const std::int64_t L = static_cast<std::int64_t>(m_ribbonLengthMM);
    for (const auto& point : rope.points)
    {
        const double xLogical =
            (L > 0) ? static_cast<double>(normalizeModI64(point.xAbs, L))
                    : static_cast<double>(point.xAbs);
        m_points.emplace_back(xLogical, static_cast<double>(point.y));
        m_pointsXAbs.push_back(point.xAbs);
    }

    rebuildSegments();
}

// ------------------------------------------------------------
// Segments rebuild
// ------------------------------------------------------------
void WorkspaceModel::rebuildSegments()
{
    m_segmentOrientations.clear();
    m_segments.clear();

    if (m_points.size() < 2)
    {
        m_crossings.clear();
        return;
    }

    for (size_t i = 1; i < m_points.size(); ++i)
    {
        QLineF seg(
            QPointF(static_cast<double>(m_pointsXAbs[i - 1]), m_points[i - 1].y()),
            QPointF(static_cast<double>(m_pointsXAbs[i]),     m_points[i].y())
        );

        m_segments.push_back(seg);

        Orientation o = computeOrientation(seg);
        m_segmentOrientations.push_back(o);
    }
    assert(m_segments.size() == m_segmentOrientations.size());

    rebuildCrossings();
}

// ------------------------------------------------------------
// Rebuild crossings (V2) ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â COHÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â°RENT CYLINDRE
// ------------------------------------------------------------
void WorkspaceModel::rebuildCrossings()
{
    struct Key
    {
        int a = -1;
        int b = -1;
        int tour = 0;

        bool operator<(const Key& o) const
        {
            if (a != o.a) return a < o.a;
            if (b != o.b) return b < o.b;
            return tour < o.tour;
        }
    };

    std::map<Key, bool> overByKey;
    for (const auto& c : m_crossings)
    {
        const int a = std::min(c.segmentAIndex, c.segmentBIndex);
        const int b = std::max(c.segmentAIndex, c.segmentBIndex);
        overByKey[Key{a, b, c.tour}] = c.newSegmentOver;
    }

    m_crossings.clear();

    const int n = static_cast<int>(m_segments.size());
    if (n < 2) return;
    if (m_ribbonLengthMM <= 0) return;
    if (static_cast<int>(m_pointsXAbs.size()) != static_cast<int>(m_points.size()))
        return;

    const std::int64_t L = static_cast<std::int64_t>(m_ribbonLengthMM);

    struct AbsSeg
    {
        QPointF a;
        QPointF b;
        double  xMin = 0.0;
        double  xMax = 0.0;
        double  xMid = 0.0;
    };

    std::vector<AbsSeg> absSegs;
    absSegs.reserve(n);

    for (int i = 0; i < n; ++i)
    {
        const int p0 = i;
        const int p1 = i + 1;

        const double x0 = static_cast<double>(m_pointsXAbs[p0]);
        const double x1 = static_cast<double>(m_pointsXAbs[p1]);
        const double y0 = m_points[p0].y();
        const double y1 = m_points[p1].y();

        AbsSeg s;
        s.a = QPointF(x0, y0);
        s.b = QPointF(x1, y1);
        s.xMin = std::min(x0, x1);
        s.xMax = std::max(x0, x1);
        s.xMid = 0.5 * (x0 + x1);
        absSegs.push_back(s);
    }

    auto sharesEndpoint = [](int i, int j) -> bool
    {
        const int i0 = i;
        const int i1 = i + 1;
        const int j0 = j;
        const int j1 = j + 1;
        return (i0 == j0) || (i0 == j1) || (i1 == j0) || (i1 == j1);
    };

    auto paramOnSegment = [](const QPointF& a, const QPointF& b, const QPointF& p) -> double
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
    };

    auto isEndpointIntersection = [&](const QLineF& l1, const QLineF& l2, const QPointF& inter) -> bool
    {
        const double t1 = paramOnSegment(l1.p1(), l1.p2(), inter);
        const double t2 = paramOnSegment(l2.p1(), l2.p2(), inter);
        const double eps = 1e-6;

        const bool end1 = (t1 <= eps) || (t1 >= 1.0 - eps);
        const bool end2 = (t2 <= eps) || (t2 >= 1.0 - eps);
        return end1 || end2;
    };

    for (int i = 0; i < n; ++i)
    {
        for (int j = i + 1; j < n; ++j)
        {
            if (sharesEndpoint(i, j))
                continue;

            const AbsSeg& si = absSegs[i];
            const AbsSeg& sj = absSegs[j];

            const double kMinD = std::floor((si.xMin - sj.xMax) / static_cast<double>(L));
            const double kMaxD = std::ceil ((si.xMax - sj.xMin) / static_cast<double>(L));

            int kMin = static_cast<int>(kMinD) - 1;
            int kMax = static_cast<int>(kMaxD) + 1;

            const int maxSpan = 9;
            const int span = kMax - kMin;
            if (span > maxSpan)
            {
                const int mid = (kMin + kMax) / 2;
                kMin = mid - (maxSpan / 2);
                kMax = kMin + maxSpan;
            }

            bool has = false;
            QPointF bestInter;
            std::int64_t bestXAbs = 0;
            double bestScore = 1e100;

            for (int k = kMin; k <= kMax; ++k)
            {
                const double shift = static_cast<double>(k) * static_cast<double>(L);

                const QLineF li(si.a, si.b);
                const QLineF lj(QPointF(sj.a.x() + shift, sj.a.y()),
                                QPointF(sj.b.x() + shift, sj.b.y()));

                QPointF inter;
                if (li.intersects(lj, &inter) == QLineF::BoundedIntersection)
                {
                    if (isEndpointIntersection(li, lj, inter))
                        continue;

                    const double score = std::fabs(inter.x() - si.xMid);
                    if (score < bestScore)
                    {
                        bestScore = score;
                        bestInter = inter;
                        bestXAbs  = static_cast<std::int64_t>(std::llround(inter.x()));
                        has = true;
                    }
                }
            }

            if (!has)
                continue;

            Crossing c;
            const std::int64_t xAbs = bestXAbs;
            const std::int64_t xMod = normalizeModI64(xAbs, L);

            c.positionMM = QPointF(static_cast<double>(xMod), bestInter.y());
            c.segmentAIndex = i;
            c.segmentBIndex = j;
            c.tour = floorDivI64(xAbs, L);

            const int a = std::min(i, j);
            const int b = std::max(i, j);
            auto it = overByKey.find(Key{a, b, c.tour});
            c.newSegmentOver = (it != overByKey.end()) ? it->second : true;

            m_crossings.push_back(c);
        }
    }
}

// ------------------------------------------------------------
// Add Point
// ------------------------------------------------------------
void WorkspaceModel::addPointMM(const QPointF& posMM)
{
    addPoint(posMM);
}

void WorkspaceModel::addPoint(const QPointF& posMM)
{
    startDesignTimeIfNeeded();

    Action action;
    action.type       = Action::Type::AddPoint;
    action.positionMM = posMM;
    action.ropeId     = activeRopeId();

    apply(action);

    m_undoStack.push(action);
    while (!m_redoStack.empty())
        m_redoStack.pop();

    rebuildSegments();

#if 0
    // DEPRECATED incremental crossing detection (kept for traceability)
    if (m_segments.size() >= 2)
    {
        const int newIndex = static_cast<int>(m_segments.size()) - 1;
        const QLineF& newSeg = m_segments[newIndex];

        for (int i = 0; i < newIndex - 1; ++i)
        {
            QPointF inter;
            if (segmentsIntersect(newSeg, m_segments[i], &inter))
            {
                Crossing c;
                c.positionMM     = inter;
                c.segmentAIndex  = i;
                c.segmentBIndex  = newIndex;
                c.newSegmentOver = true;

                std::int64_t xCrossAbs = m_pointsXAbs[newIndex];
                c.tour = static_cast<int>(xCrossAbs / m_ribbonLengthMM);

                m_crossings.push_back(c);
            }
        }
    }
#endif
}

void WorkspaceModel::addPointAbs(const QPointF& posMM, std::int64_t xAbsMM)
{
    startDesignTimeIfNeeded();

    Action action;
    action.type = Action::Type::AddPoint;
    action.positionMM = posMM;
    action.ropeId = activeRopeId();
    action.hasAbsoluteX = true;
    action.absoluteXMM = xAbsMM;

    apply(action);

    m_undoStack.push(action);
    while (!m_redoStack.empty())
        m_redoStack.pop();

    rebuildSegments();
}

// ------------------------------------------------------------
// Apply / Revert
// ------------------------------------------------------------
void WorkspaceModel::apply(const Action& action)
{
    if (action.type == Action::Type::AddPoint)
    {
        const std::int64_t xAbs = resolveAbsoluteX(action);
        const std::int32_t y = static_cast<std::int32_t>(std::llround(action.positionMM.y()));

        m_points.push_back(action.positionMM);
        m_pointsXAbs.push_back(xAbs);

        // MIGRATION-PARALLEL: double-ÃƒÆ’Ã‚Â©criture legacy + TopologyStore
        m_topologyStore.appendAbsPointToRope(static_cast<Domain::RopeId>(action.ropeId), xAbs, y);
        m_topologyStore.rebuildDerivedGeometry();
    }
}

void WorkspaceModel::revert(const Action& action)
{
    if (action.type == Action::Type::AddPoint && !m_points.empty())
    {
        m_points.pop_back();
        if (!m_pointsXAbs.empty())
            m_pointsXAbs.pop_back();

        // MIGRATION-PARALLEL
        m_topologyStore.popLastPointFromRope(static_cast<Domain::RopeId>(action.ropeId));
    }
    rebuildSegments();
}

// ------------------------------------------------------------
// Undo / Redo
// ------------------------------------------------------------
bool WorkspaceModel::canUndo() const { return !m_undoStack.empty(); }
bool WorkspaceModel::canRedo() const { return !m_redoStack.empty(); }

void WorkspaceModel::undo()
{
    if (!canUndo()) return;

    Action action = m_undoStack.top();
    m_undoStack.pop();

    revert(action);
    m_redoStack.push(action);
}

void WorkspaceModel::redo()
{
    if (!canRedo()) return;

    Action action = m_redoStack.top();
    m_redoStack.pop();

    apply(action);
    m_undoStack.push(action);

    rebuildSegments();
}

// ------------------------------------------------------------
// Ribbon
// ------------------------------------------------------------
int WorkspaceModel::ribbonLengthMM() const { return m_ribbonLengthMM; }
int WorkspaceModel::ribbonOffsetMM() const { return m_ribbonOffsetMM; }

void WorkspaceModel::rotateRibbonMM(int deltaMM)
{
    m_ribbonOffsetMM = (m_ribbonOffsetMM + deltaMM) % m_ribbonLengthMM;
    if (m_ribbonOffsetMM < 0)
        m_ribbonOffsetMM += m_ribbonLengthMM;

    m_topologyStore.setRibbonOffsetMM(m_ribbonOffsetMM);
}

void WorkspaceModel::setRibbonLengthMM(int value)
{
    if (value < 140)
        value = 140;

    m_ribbonLengthMM = value;
    m_ribbonOffsetMM = static_cast<int>(normalizeModI64(m_ribbonOffsetMM, m_ribbonLengthMM));

    rebuildPointsXAbs();
    rebuildSegments();

    m_topologyStore.setRibbonLengthMM(m_ribbonLengthMM);
    m_topologyStore.setRibbonOffsetMM(m_ribbonOffsetMM);
    syncTopologyStoreFromLegacy();
}

void WorkspaceModel::setRibbonOffsetMM(int v)
{
    m_ribbonOffsetMM = v % m_ribbonLengthMM;
    if (m_ribbonOffsetMM < 0)
        m_ribbonOffsetMM += m_ribbonLengthMM;

    m_topologyStore.setRibbonOffsetMM(m_ribbonOffsetMM);
}

// ------------------------------------------------------------
// Save
// ------------------------------------------------------------
bool WorkspaceModel::saveToFile(const QString& filePath) const
{
    QJsonObject root;

    root["format"]  = "LogiKnotting";
    root["version"] = 3;

    QJsonObject ribbon;
    ribbon["lengthMM"] = m_ribbonLengthMM;
    ribbon["offsetMM"] = m_ribbonOffsetMM;
    root["ribbon"] = ribbon;

    root["design_time_seconds"] =
        static_cast<qint64>(designTimeSeconds());

    root["total_work_seconds"] = static_cast<qint64>(m_totalWorkSeconds);

    QJsonObject audit;
    audit["creator_author_id"] = m_creatorAuthorId;
    audit["created_at_utc"] = m_createdAtUtcIso;
    audit["last_modified_by_author_id"] = m_lastModifiedByAuthorId;
    audit["last_modified_at_utc"] = m_lastModifiedAtUtcIso;
    audit["total_work_seconds"] = static_cast<qint64>(m_totalWorkSeconds);

    QJsonArray history;
    for (const FileHistoryEntry& e : m_fileHistory)
    {
        QJsonObject h;
        h["action"] = e.action;
        h["author_id"] = e.authorId;
        h["at_utc"] = e.atUtcIso;
        h["work_seconds"] = static_cast<qint64>(e.workSeconds);
        history.append(h);
    }
    audit["history"] = history;
    root["file_audit"] = audit;


    QJsonArray pts;

    for (const QPointF& p : m_points)
    {
        QJsonArray xy;
        xy.append(p.x());
        xy.append(p.y());
        pts.append(xy);
    }

    root["points"] = pts;

    QJsonArray crossingsArray;

    for (const Crossing& c : m_crossings)
    {
        QJsonObject co;

        co["x"]    = c.positionMM.x();
        co["y"]    = c.positionMM.y();
        co["a"]    = c.segmentAIndex;
        co["b"]    = c.segmentBIndex;
        co["over"] = c.newSegmentOver;
        co["tour"] = c.tour;

        crossingsArray.append(co);
    }

    root["crossings"] = crossingsArray;

    // MIGRATION-PARALLEL: extension V2+ optionnelle pour multi-cordes
    const auto& topo = m_topologyStore.snapshot();
    root["active_rope_id"] = static_cast<int>(topo.activeRopeId);

    QJsonArray ropesArray;
    for (const auto& rope : topo.ropes)
    {
        QJsonObject ro;
        ro["id"] = static_cast<int>(rope.ropeId);
        ro["color"] = rope.color.name(QColor::HexRgb);

        QJsonArray pointsAbs;
        for (const auto& p : rope.points)
        {
            QJsonArray xy;
            xy.append(static_cast<qint64>(p.xAbs));
            xy.append(static_cast<int>(p.y));
            pointsAbs.append(xy);
        }

        ro["points_abs"] = pointsAbs;
        ropesArray.append(ro);
    }
    root["ropes"] = ropesArray;

    QJsonArray topologyCrossingsArray;
    for (const auto& crossing : topo.crossings)
    {
        QJsonObject co;
        co["min_rope_id"] = static_cast<int>(crossing.key.sMin.ropeId);
        co["min_seg_index"] = crossing.key.sMin.segIndex;
        co["max_rope_id"] = static_cast<int>(crossing.key.sMax.ropeId);
        co["max_seg_index"] = crossing.key.sMax.segIndex;
        co["turn"] = crossing.key.turn;
        co["s2_over_s1"] = crossing.s2OverS1;
        topologyCrossingsArray.append(co);
    }
    root["topology_crossings"] = topologyCrossingsArray;

    if (filePath.endsWith(QStringLiteral(".lkw"), Qt::CaseInsensitive)
        && !m_sketchOverlayState.isEmpty())
    {
        root["sketch_overlay"] = m_sketchOverlayState;
    }

    const QString integrityNonce = makeIntegrityNonce();
    QJsonObject integrity;
    integrity["alg"] = QString::fromLatin1(kIntegrityAlgorithm);
    integrity["nonce"] = integrityNonce;
    integrity["sig"] = computeIntegritySignature(root, integrityNonce);
    root["integrity"] = integrity;

    QJsonDocument doc(root);

    QSaveFile f(filePath);
    if (!f.open(QIODevice::WriteOnly))
        return false;

    f.write(doc.toJson(QJsonDocument::Indented));

    return f.commit();
}

// ------------------------------------------------------------
// Orientation
// ------------------------------------------------------------
Model::Orientation WorkspaceModel::computeOrientation(const QLineF& seg) const
{
    double angle = seg.angle(); // Qt: 0ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â° = East, CCW positive

    int quant = static_cast<int>(std::round(angle / 45.0)) % 8;

    switch (quant)
    {
    case 0:
    case 4: return Orientation::Deg0;

    case 1:
    case 5: return Orientation::Deg45;

    case 2:
    case 6: return Orientation::Deg90;

    case 3:
    case 7: return Orientation::Deg135;

    default: return Orientation::Deg0;
    }
}

// ------------------------------------------------------------
// Load
// ------------------------------------------------------------
bool WorkspaceModel::loadFromFile(const QString& filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject())
        return false;

    QJsonObject root = doc.object();

    const QString fileFormat = root["format"].toString();
    if (fileFormat != "LogiKnotting")
        return false;

    if (root["version"].toInt() != 3)
        return false;

    QJsonObject verifiedPayload;
    if (!extractAndVerifyIntegrity(root, verifiedPayload))
        return false;
    root = verifiedPayload;

    QJsonObject ribbon = root["ribbon"].toObject();

    int ribbonLength = ribbon["lengthMM"].toInt();
    int ribbonOffset = ribbon["offsetMM"].toInt();

    m_ribbonLengthMM = ribbonLength;
    m_ribbonOffsetMM = ribbonOffset;

    m_designTimeSeconds =
        static_cast<std::int64_t>(root["design_time_seconds"].toVariant().toLongLong());

    m_points.clear();
    m_crossings.clear();
    m_segments.clear();
    m_segmentOrientations.clear();
    m_pointsXAbs.clear();
    m_fileHistory.clear();
    m_sketchOverlayState = QJsonObject();

    m_creatorAuthorId.clear();
    m_createdAtUtcIso.clear();
    m_lastModifiedByAuthorId.clear();
    m_lastModifiedAtUtcIso.clear();
    m_totalWorkSeconds = static_cast<std::int64_t>(root["total_work_seconds"].toVariant().toLongLong());

    const QJsonObject audit = root["file_audit"].toObject();
    if (!audit.isEmpty())
    {
        m_creatorAuthorId = canonicalAuthorId(audit["creator_author_id"].toString());
        m_createdAtUtcIso = audit["created_at_utc"].toString().trimmed();
        m_lastModifiedByAuthorId = canonicalAuthorId(audit["last_modified_by_author_id"].toString());
        m_lastModifiedAtUtcIso = audit["last_modified_at_utc"].toString().trimmed();

        const std::int64_t auditTotal = static_cast<std::int64_t>(audit["total_work_seconds"].toVariant().toLongLong());
        if (auditTotal > 0)
            m_totalWorkSeconds = auditTotal;

        const QJsonArray history = audit["history"].toArray();
        for (qsizetype i = 0; i < history.size(); ++i)
        {
            const QJsonObject h = history.at(i).toObject();
            FileHistoryEntry entry;
            entry.action = h["action"].toString().trimmed().toLower();
            entry.authorId = canonicalAuthorId(h["author_id"].toString());
            entry.atUtcIso = h["at_utc"].toString().trimmed();
            entry.workSeconds = static_cast<std::int64_t>(h["work_seconds"].toVariant().toLongLong());
            if (entry.workSeconds < 0)
                entry.workSeconds = 0;

            m_fileHistory.push_back(entry);
        }
    }

    if (m_totalWorkSeconds <= 0)
        m_totalWorkSeconds = m_designTimeSeconds;

    m_designTimeRunning = false;
    m_designTimeStartMS = 0;
    m_lastDesignActivityMS = 0;

    QJsonArray pts = root["points"].toArray();

    for (qsizetype i = 0; i < pts.size(); ++i)
    {
        const QJsonValue& v = pts.at(i);

        QJsonArray xy = v.toArray();
        if (xy.size() >= 2)
        {
            m_points.emplace_back(
                xy.at(0).toDouble(),
                xy.at(1).toDouble()
            );
        }
    }

    QJsonArray crossingsArray = root["crossings"].toArray();

    for (qsizetype i = 0; i < crossingsArray.size(); ++i)
    {
        const QJsonObject co = crossingsArray.at(i).toObject();

        Crossing c;

        c.positionMM.setX(co["x"].toDouble());
        c.positionMM.setY(co["y"].toDouble());
        c.segmentAIndex  = co["a"].toInt();
        c.segmentBIndex  = co["b"].toInt();
        c.newSegmentOver = co["over"].toBool(true);
        c.tour           = co["tour"].toInt();

        m_crossings.push_back(c);
    }

    std::map<Domain::CrossingKey, bool> topologyCrossingStates;
    const QJsonArray topologyCrossingsArray = root["topology_crossings"].toArray();
    for (qsizetype i = 0; i < topologyCrossingsArray.size(); ++i)
    {
        const QJsonObject co = topologyCrossingsArray.at(i).toObject();
        Domain::CrossingKey key;
        key.sMin.ropeId = static_cast<Domain::RopeId>(co["min_rope_id"].toInt(0));
        key.sMin.segIndex = co["min_seg_index"].toInt(-1);
        key.sMax.ropeId = static_cast<Domain::RopeId>(co["max_rope_id"].toInt(0));
        key.sMax.segIndex = co["max_seg_index"].toInt(-1);
        key.turn = co["turn"].toInt(0);
        topologyCrossingStates[key] = co["s2_over_s1"].toBool(true);
    }

    rebuildPointsXAbs();
    rebuildSegments();

    m_topologyStore.setRibbonLengthMM(m_ribbonLengthMM);
    m_topologyStore.setRibbonOffsetMM(m_ribbonOffsetMM);

    const QJsonArray ropesArray = root["ropes"].toArray();
    if (!ropesArray.isEmpty())
    {
        m_topologyStore.clearPointsOnly();

        for (qsizetype i = 0; i < ropesArray.size(); ++i)
        {
            const QJsonObject ro = ropesArray.at(i).toObject();
            const int rid = ro["id"].toInt(0);
            const QJsonArray pointsAbs = ro["points_abs"].toArray();

            for (qsizetype j = 0; j < pointsAbs.size(); ++j)
            {
                const QJsonArray xy = pointsAbs.at(j).toArray();
                if (xy.size() < 2)
                    continue;

                const std::int64_t xAbs = static_cast<std::int64_t>(xy.at(0).toVariant().toLongLong());
                const std::int32_t y = xy.at(1).toInt();
                m_topologyStore.appendAbsPointToRope(static_cast<Domain::RopeId>(rid), xAbs, y);
            }
        }

        m_topologyStore.rebuildDerivedGeometry();
        m_topologyStore.setActiveRopeId(root["active_rope_id"].toInt(0));
    }
    else
    {
        syncTopologyStoreFromLegacy();
    }

    for (auto it = topologyCrossingStates.begin(); it != topologyCrossingStates.end(); ++it)
        m_topologyStore.setCrossingOver(it->first, it->second);

    if (filePath.endsWith(QStringLiteral(".lkw"), Qt::CaseInsensitive))
        m_sketchOverlayState = root["sketch_overlay"].toObject();

    return true;
}

// ------------------------------------------------------------
// Resize ribbon
// ------------------------------------------------------------
bool WorkspaceModel::canResizeRibbon(int deltaMM) const
{
    const int newLength = m_ribbonLengthMM + deltaMM;

    if (newLength < 140)
        return false;

    double maxX = 0.0;
    for (const QPointF& p : m_points)
        maxX = std::max(maxX, p.x());

    if (newLength <= static_cast<int>(std::ceil(maxX)))
        return false;

    return true;
}
void WorkspaceModel::clear()
{
    m_points.clear();
    m_pointsXAbs.clear();
    m_segments.clear();
    m_segmentOrientations.clear();
    m_crossings.clear();

    while (!m_undoStack.empty())
        m_undoStack.pop();

    while (!m_redoStack.empty())
        m_redoStack.pop();

    m_ribbonLengthMM = 280;
    m_ribbonOffsetMM = 0;

    // MIGRATION-PARALLEL
    m_topologyStore.clear();
    m_topologyStore.setRibbonLengthMM(m_ribbonLengthMM);
    m_topologyStore.setRibbonOffsetMM(m_ribbonOffsetMM);


    m_creatorAuthorId.clear();
    m_createdAtUtcIso.clear();
    m_lastModifiedByAuthorId.clear();
    m_lastModifiedAtUtcIso.clear();
    m_totalWorkSeconds = 0;
    m_fileHistory.clear();
    m_sketchOverlayState = QJsonObject();
    resetDesignTime();
}
void WorkspaceModel::resizeRibbonMM(int deltaMM)
{
    if (!canResizeRibbon(deltaMM))
        return;

    m_ribbonLengthMM += deltaMM;

    if (m_ribbonOffsetMM >= m_ribbonLengthMM)
        m_ribbonOffsetMM %= m_ribbonLengthMM;

    rebuildPointsXAbs();
    rebuildSegments();

    m_topologyStore.setRibbonLengthMM(m_ribbonLengthMM);
}

bool WorkspaceModel::wrapIsDone() const
{
    if (m_points.size() < 2)
        return false;

    const double L = static_cast<double>(m_ribbonLengthMM);

    for (size_t i = 1; i < m_points.size(); ++i)
    {
        const double x0 = m_points[i - 1].x();
        const double x1 = m_points[i].x();

        double d = x1 - x0;

        if (std::abs(d) > L * 0.5)
            return true;
    }

    return false;
}
// ------------------------------------------------------------
// Bights count
// ------------------------------------------------------------
int WorkspaceModel::bightCount() const
{
    if (m_points.size() < 2)
        return 0;

    int count = 0;

    const double L = static_cast<double>(m_ribbonLengthMM);

    for (size_t i = 1; i < m_points.size(); ++i)
    {
        const double x0 = m_points[i - 1].x();
        const double x1 = m_points[i].x();

        const double d = x1 - x0;

        // franchissement de jonction
        if (std::abs(d) > L * 0.5)
            count++;
    }

    return count;
}
}

// ============================================================
// End Of File
// ============================================================



