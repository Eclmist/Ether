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
    class RhiCommandAllocator
    {
    public:
        RhiCommandAllocator(RhiCommandType type) : m_Type(type) {}
        virtual ~RhiCommandAllocator() = default;

        inline RhiCommandType GetType() const { return m_Type; }

    public:
        virtual void Reset() const = 0;

    private:
        const RhiCommandType m_Type;
    };
}

