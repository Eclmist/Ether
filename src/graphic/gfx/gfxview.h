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
#include "graphic/hal/dx12includes.h"

class GfxView : NonCopyable
{
public:
    GfxView();
    ~GfxView() = default;

    inline D3D12_VIEWPORT GetViewport() const { return m_Viewport; };
    inline D3D12_RECT GetScissorRect() const { return m_ScissorRect; };

    void UpdateView(uint32_t width, uint32_t height);

private:
    D3D12_VIEWPORT m_Viewport;
    D3D12_RECT m_ScissorRect;
};
