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

#include "graphic/hal/dx12commandlist.h"

void DX12CommandList::CreateCommandList()
{
    if (m_CommandList != nullptr)
        return;

    CreateCommandAllocator();
    ThrowIfFailed(m_Device->CreateCommandList(0, m_Type, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList)));
}

void DX12CommandList::CreateCommandAllocator()
{
    if (m_CommandAllocator != nullptr)
        return;

    ThrowIfFailed(m_Device->CreateCommandAllocator(m_Type, IID_PPV_ARGS(&m_CommandAllocator)));
}
