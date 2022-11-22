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

#ifdef ETH_GRAPHICS_DX12

#include "graphics/rhi/dx12/dx12commandallocator.h"

Ether::Graphics::Dx12CommandAllocator::Dx12CommandAllocator(RhiCommandType type)
    : RhiCommandAllocator(type)
{
}

void Ether::Graphics::Dx12CommandAllocator::Reset() const
{
    HRESULT hr = m_Allocator->Reset();
	if (FAILED(hr))
		LogGraphicsError("Failed to reset command allocator");
}

#endif // ETH_GRAPHICS_DX12

