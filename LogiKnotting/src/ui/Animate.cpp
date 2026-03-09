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
// File        : src/ui/Animate.cpp
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#include "Animate.h"

#include <algorithm>

namespace Ui
{
Animate::Animate(QObject* parent)
    : QObject(parent)
{
    m_timer.setInterval(TickMs);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &Animate::onTick);
}

void Animate::setTotalSegments(std::size_t totalSegments)
{
    m_totalSegments = totalSegments;
    if (m_segmentIndex > m_totalSegments)
        m_segmentIndex = m_totalSegments;

    emit frameChanged(m_segmentIndex, 1.0);
}

void Animate::setMode(Mode mode)
{
    m_mode = mode;
    m_elapsedMs = 0;
    emit frameChanged(m_segmentIndex, 1.0);
}

void Animate::start()
{
    if (m_running)
        return;

    if (m_segmentIndex >= m_totalSegments)
        rewind();

    m_running = true;
    m_timer.start();

    emit frameChanged(m_segmentIndex, 1.0);
}

void Animate::stop()
{
    if (!m_running)
        return;

    m_running = false;
    m_timer.stop();

    emit frameChanged(m_segmentIndex, 1.0);
}

void Animate::rewind()
{
    m_segmentIndex = 0;
    m_elapsedMs = 0;
    emit frameChanged(m_segmentIndex, 1.0);
}

void Animate::stepOnce()
{
    if (m_segmentIndex < m_totalSegments)
        m_segmentIndex++;

    m_elapsedMs = 0;
    emit frameChanged(m_segmentIndex, 1.0);

    if (m_segmentIndex >= m_totalSegments)
        emit finished();
}

double Animate::smoothstep(double t)
{
    t = std::clamp(t, 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

int Animate::segmentDurationMsClamped() const
{
    return std::max(MinDurationMs, SegmentDurationMs);
}

void Animate::onTick()
{
    if (!m_running)
        return;

    if (m_segmentIndex >= m_totalSegments)
    {
        stop();
        emit finished();
        return;
    }

    if (m_mode == Mode::Simple)
    {
        m_elapsedMs += TickMs;
        if (m_elapsedMs < SimpleStepPauseMs)
            return;

        m_elapsedMs = 0;
        m_segmentIndex++;

        emit frameChanged(m_segmentIndex, 1.0);

        if (m_segmentIndex >= m_totalSegments)
        {
            stop();
            emit finished();
        }
        return;
    }

    // Mode Serpent : progression continue du segment courant
    const int duration = segmentDurationMsClamped();
    m_elapsedMs += TickMs;

    const double t = static_cast<double>(m_elapsedMs) / static_cast<double>(duration);
    const double p = smoothstep(t);

    emit frameChanged(m_segmentIndex, p);

    if (t >= 1.0)
    {
        m_elapsedMs = 0;
        m_segmentIndex++;

        emit frameChanged(m_segmentIndex, 1.0);

        if (m_segmentIndex >= m_totalSegments)
        {
            stop();
            emit finished();
        }
    }
}

}

// ============================================================
// End Of File
// ============================================================
