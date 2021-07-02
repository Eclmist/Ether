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

class TextureResource : public GPUResource
{
public:
    TextureResource(const std::wstring& name, D3D12_RESOURCE_DESC desc, D3D12_CLEAR_VALUE clearColor);
    TextureResource(const std::wstring& name, ID3D12Resource* swapChainBufferResource);
    ~TextureResource() = default;

public:
    static D3D12_RESOURCE_DESC CreateResourceDesc(
        uint32_t width,
        uint32_t height,
        uint32_t numMips,
        DXGI_FORMAT format,
        D3D12_RESOURCE_FLAGS flags);

    const D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const { return m_SRVHandle; }
    const D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const { return m_RTVHandle; }

protected:
    void CreateDescriptorHeaps();
    void CreateViews();

protected:
    uint32_t m_Width;
    uint32_t m_Height;
    uint32_t m_NumMips;
    
    DXGI_FORMAT m_Format;

    D3D12_CPU_DESCRIPTOR_HANDLE m_RTVHandle;
    D3D12_CPU_DESCRIPTOR_HANDLE m_SRVHandle;

    wrl::ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
    wrl::ComPtr<ID3D12DescriptorHeap> m_SRVDescriptorHeap;
};

ETH_NAMESPACE_END