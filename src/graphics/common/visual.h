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

namespace Ether::Graphics
{
struct VisualBatch;
class Mesh;
class Material;

struct ETH_GRAPHIC_DLL Visual
{
    Mesh* m_Mesh;
    Material* m_Material;
    bool m_Culled;

    bool operator==(const Visual& other) const
    {
        if (m_Mesh != other.m_Mesh)
            return false;
        if (m_Material != other.m_Material)
            return false;
        return true;
    }
};
} // namespace Ether::Graphics
