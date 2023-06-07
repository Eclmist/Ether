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

#include "graphics/rhi/dx12/dx12raytracingshaderbindingtable.h"

#ifdef ETH_GRAPHICS_DX12

Ether::Graphics::Dx12RaytracingShaderBindingTable::Dx12RaytracingShaderBindingTable(
    const char* name,
    uint32_t maxEntrySize,
    uint32_t numEntries)
    : RhiRaytracingShaderBindingTable(name, maxEntrySize, numEntries)
{
}

Ether::Graphics::RhiGpuAddress Ether::Graphics::Dx12RaytracingShaderBindingTable::GetGpuAddress() const
{
    return m_Buffer->GetGpuAddress();
}

#endif // ETH_GRAPHICS_DX12
