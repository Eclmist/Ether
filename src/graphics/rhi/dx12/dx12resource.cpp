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

#include "graphics/rhi/dx12/dx12resource.h"
#include "graphics/rhi/dx12/dx12translation.h"
#include <locale>
#include <codecvt>
#include <string>

Ether::Graphics::Dx12Resource::Dx12Resource(const std::string& name)
    : RhiResource(name)
{
}

Ether::Graphics::RhiGpuAddress Ether::Graphics::Dx12Resource::GetGpuAddress() const
{
    return { m_Resource->GetGPUVirtualAddress() };
}

void Ether::Graphics::Dx12Resource::Map(void** mappedAddr) const
{
    HRESULT hr = m_Resource->Map(0, nullptr, mappedAddr);

    if (FAILED(hr))
    {
        LogGraphicsError("Failed to map GPU resource (%s)", m_Resource->GetGPUVirtualAddress());
        throw std::runtime_error("Failed to map GPU resource");
    }
}

void Ether::Graphics::Dx12Resource::Unmap() const
{
    m_Resource->Unmap(0, nullptr);
}

#endif // ETH_GRAPHICS_DX12

