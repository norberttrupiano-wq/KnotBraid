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
// File        : src/core/CylinderTopology.cpp
// Created     : 2026-03-06
// Updated     : 2026-03-07
// Description :
// ============================================================
#include "CylinderTopology.h"

#include <cstdlib>

namespace Core
{
std::int32_t CylinderTopology::wrapMod(std::int64_t xAbs, std::int32_t length)
{
    if (length <= 0) return 0;
    const std::int64_t m = static_cast<std::int64_t>(length);
    const std::int64_t r = xAbs % m;
    return static_cast<std::int32_t>(r < 0 ? (r + m) : r);
}

std::int64_t CylinderTopology::unwrapNearest(std::int32_t xMod, std::int64_t anchorXAbs, std::int32_t length)
{
    if (length <= 0) return static_cast<std::int64_t>(xMod);

    const std::int64_t L = static_cast<std::int64_t>(length);
    const std::int64_t xm = static_cast<std::int64_t>(wrapMod(xMod, length));
    const std::int64_t k0 = anchorXAbs / L;

    std::int64_t best = xm + k0 * L;
    std::int64_t bestDist = std::llabs(best - anchorXAbs);

    for (int dk = -1; dk <= 1; ++dk)
    {
        const std::int64_t cand = xm + (k0 + dk) * L;
        const std::int64_t dist = std::llabs(cand - anchorXAbs);
        if (dist < bestDist)
        {
            best = cand;
            bestDist = dist;
        }
    }

    return best;
}

std::int32_t CylinderTopology::shortestDeltaMod(std::int32_t fromMod, std::int32_t toMod, std::int32_t length)
{
    if (length <= 0) return toMod - fromMod;

    int d = (toMod - fromMod) % length;
    if (d < 0) d += length;
    if (d > length / 2) d -= length;
    return d;
}

std::int32_t CylinderTopology::turnOf(std::int64_t xAbs, std::int32_t length)
{
    if (length <= 0) return 0;
    const std::int64_t L = static_cast<std::int64_t>(length);
    if (xAbs >= 0) return static_cast<std::int32_t>(xAbs / L);
    return static_cast<std::int32_t>(- ((-xAbs + L - 1) / L));
}
}

// ============================================================
// End Of File
// ============================================================
