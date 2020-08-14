/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel Van Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "system/system.h"

struct GfxStaleDescriptorInfo
{
    GfxStaleDescriptorInfo(uint32_t offset, uint32_t size, uint32_t graphicFrameNumber)
        : m_Offset(offset)
        , m_Size(size)
        , m_GraphicFrameNumber(graphicFrameNumber) {};

    uint32_t m_Offset;
    uint32_t m_Size;
    uint32_t m_GraphicFrameNumber;
};
