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
// File        : src/ui/Animate.h
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#pragma once

#include <QObject>
#include <QTimer>
#include <cstddef>

namespace Ui
{
class Animate final : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------
    // Réglages (VITESSES DANS LE .H)
    // ------------------------------------------------------------
    static constexpr int TickMs             = 16;   // 16 ≈ 60 fps
    static constexpr int SegmentDurationMs  = 300;  // Durée d'un segment (mode Serpent)
    static constexpr int SimpleStepPauseMs  = 60;   // Pause entre 2 points (mode Simple)
    static constexpr int MinDurationMs      = 40;   // Sécurité

    enum class Mode
    {
        Simple,   // point par point, segment instantané
        Serpent   // point par point, segment progressif (easing)
    };

public:
    explicit Animate(QObject* parent = nullptr);
    ~Animate() override = default;

    void setTotalSegments(std::size_t totalSegments);
    void setMode(Mode mode);

    void start();
    void stop();
    void rewind();
    void stepOnce();

    bool running() const { return m_running; }
    Mode mode() const { return m_mode; }

signals:
    // segmentIndex : nombre de segments COMPLETS déjà validés (0..totalSegments)
    // progress     : [0..1] progression du segment courant (Serpent), 1 en Simple
    void frameChanged(std::size_t segmentIndex, double progress);

    void finished();

private slots:
    void onTick();

private:
    static double smoothstep(double t);
    int segmentDurationMsClamped() const;

private:
    QTimer      m_timer;
    std::size_t m_totalSegments = 0;

    bool        m_running = false;
    Mode        m_mode    = Mode::Simple;

    // segment courant (Serpent)
    std::size_t m_segmentIndex = 0;  // segments complétés
    int         m_elapsedMs    = 0;  // progression du segment en cours
};

}

// ============================================================
// End Of File
// ============================================================
