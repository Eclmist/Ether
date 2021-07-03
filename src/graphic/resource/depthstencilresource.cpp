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

#include "depthstencilresource.h"

ETH_NAMESPACE_BEGIN

DepthStencilResource::DepthStencilResource(const std::wstring & name, D3D12_RESOURCE_DESC desc)
    : m_Width((uint32_t)desc.Width)
    , m_Height(desc.Height)
    , m_Format(desc.Format)
{
    D3D12_CLEAR_VALUE optimizedClearValue = {};
    optimizedClearValue.Format = desc.Format;
    optimizedClearValue.DepthStencil = { 1.0f, 0 };

    ASSERT_SUCCESS(g_GraphicDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &optimizedClearValue, IID_PPV_ARGS(&m_Resource)));

    CreateDescriptorHeaps();
    CreateViews();
}

D3D12_RESOURCE_DESC DepthStencilResource::CreateResourceDesc(
    uint32_t width, uint32_t height, DXGI_FORMAT format)
{
    D3D12_RESOURCE_DESC desc = {};
    desc.Alignment = 0;
    desc.DepthOrArraySize = 1;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    desc.Format = format;
    desc.Height = height;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.MipLevels = 0;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Width = width;
    return desc;
}

void DepthStencilResource::CreateDescriptorHeaps()
{
    // TODO: This belongs in a proper allocator. 
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = 1;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ASSERT_SUCCESS(g_GraphicDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_DSVDescriptorHeap)));
    m_DSVHandle = m_DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

void DepthStencilResource::CreateViews()
{
    D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc = {};
    DSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
    DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    DSVDesc.Texture2D.MipSlice = 0;
    DSVDesc.Flags = D3D12_DSV_FLAG_NONE;
    g_GraphicDevice->CreateDepthStencilView(m_Resource.Get(), &DSVDesc, m_DSVHandle);
}

ETH_NAMESPACE_END

