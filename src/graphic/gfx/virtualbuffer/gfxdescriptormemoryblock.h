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

struct GfxDescriptorMemoryBlock
{
    GfxDescriptorMemoryBlock(uint32_t size)
        : m_Size(size) {};

    uint32_t m_Size;
    std::multimap<uint32_t, std::map<uint32_t, GfxDescriptorMemoryBlock>::iterator>::iterator m_BlockLocation;
};
