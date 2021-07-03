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

#include "gpuresource.h"

ETH_NAMESPACE_BEGIN

class DepthStencilResource : public GPUResource
{
public:
    DepthStencilResource(const std::wstring& name, D3D12_RESOURCE_DESC desc);
    ~DepthStencilResource() = default;

public:
    static D3D12_RESOURCE_DESC CreateResourceDesc(
        uint32_t width,
        uint32_t height,
        DXGI_FORMAT format);

    const D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const { return m_DSVHandle; }

protected:
    void CreateDescriptorHeaps();
    void CreateViews();

protected:
    uint32_t m_Width;
    uint32_t m_Height;
    
    DXGI_FORMAT m_Format;

    D3D12_CPU_DESCRIPTOR_HANDLE m_DSVHandle;
    wrl::ComPtr<ID3D12DescriptorHeap> m_DSVDescriptorHeap;
};

ETH_NAMESPACE_END