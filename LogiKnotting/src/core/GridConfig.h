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
// File        : src/core/GridConfig.h
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#ifndef LogiKnots_GRIDCONFIG_H
#define LogiKnots_GRIDCONFIG_H

namespace GridConfig
{
// ============================================================
// 1. TAILLE DE GRILLE (mm conceptuels)
// ============================================================

static constexpr int Small  = 1;
static constexpr int Medium = 20;
static constexpr int Large  = 30;

// Référence métrique unique
static constexpr int Size = Small;

// ============================================================
// 2. RÈGLES — ÉPAISSEURS LOGIQUES (mm)
// ============================================================

static constexpr int RuleRedMM    = 0;
static constexpr int RuleGreenMM  = 0;
static constexpr int RuleBlueMM   = 3;
static constexpr int RuleBottomMM = 0;

// ============================================================
// 3. JONCTION (V2) — ZÉRO LOGIQUE
// ============================================================

static constexpr int JonctionCorrectionMM = 0;
static constexpr int JonctionCorrectionPx = JonctionCorrectionMM * Size;

static constexpr int JonctionXMM = 0;
static constexpr int JonctionXPx = JonctionCorrectionPx;

// Alias de confort
static constexpr int JonctionX = JonctionXPx;

// ============================================================
// 4. CONVERSION MM → PIXELS SCÈNE
// ============================================================

static constexpr int EpaisseurRegleBleue  = RuleBlueMM   ;

// ============================================================
// 5. PARAMÈTRES DE GRILLE
// ============================================================

static constexpr int MajorEvery = 5;
static constexpr int Half       = 0.5 ; //Size / 2;

static constexpr int NodeRadius    = Half - 1;
static constexpr int SegmentWidth  = Half - 1;
static constexpr int SnapTolerance = Size / 4;

// ============================================================
// 6. SCÈNE
// ============================================================

static constexpr int SceneExtent = 40000;

} // namespace GridConfig

#endif

// ============================================================
// End Of File
// ============================================================
