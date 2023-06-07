/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2023 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "graphics/pch.h"
#include "graphics/common/visual.h"

namespace Ether::Graphics
{
struct ETH_GRAPHIC_DLL VisualBatch
{
    std::vector<Visual> m_Visuals;

    // Other batch related data will go in here in the future

    bool operator==(const VisualBatch& other) const
    {
        if (m_Visuals.size() != other.m_Visuals.size())
            return false;

        for (uint32_t i = 0; i < m_Visuals.size(); ++i)
        {
            if (m_Visuals[i] != other.m_Visuals[i])
                return false;
        }

        return true;
    }
};
} // namespace Ether::Graphics
