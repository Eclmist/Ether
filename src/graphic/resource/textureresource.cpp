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

#include "textureresource.h"

ETH_NAMESPACE_BEGIN

TextureResource::TextureResource(const std::wstring & name, D3D12_RESOURCE_DESC desc, D3D12_CLEAR_VALUE clearColor)
    : m_Width((uint32_t)desc.Width)
    , m_Height(desc.Height)
    , m_NumMips(desc.MipLevels)
{
    ASSERT_SUCCESS(g_GraphicDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &desc, D3D12_RESOURCE_STATE_COMMON,
        &clearColor, IID_PPV_ARGS(&m_Resource)));

    CreateDescriptorHeaps();
    CreateViews();
}

TextureResource::TextureResource(const std::wstring& name, ID3D12Resource* swapChainBufferResource)
{
    D3D12_RESOURCE_DESC desc = swapChainBufferResource->GetDesc();
    m_Resource.Attach(swapChainBufferResource);
    m_CurrentState = D3D12_RESOURCE_STATE_PRESENT;
    m_Width = (uint32_t)desc.Width;
    m_Height = desc.Height;
    m_Format = desc.Format;
    m_Resource->SetName(name.c_str());

    CreateDescriptorHeaps();
    m_RTVHandle = m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    g_GraphicDevice->CreateRenderTargetView(m_Resource.Get(), nullptr, m_RTVHandle);
}

D3D12_RESOURCE_DESC TextureResource::CreateResourceDesc(
    uint32_t width, uint32_t height, uint32_t numMips, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags)
{
    D3D12_RESOURCE_DESC desc = {};
    desc.Alignment = 0;
    desc.DepthOrArraySize = 1;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Flags = flags;
    desc.Format = format;
    desc.Height = height;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.MipLevels = numMips;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Width = width;
    return desc;
}

void TextureResource::CreateDescriptorHeaps()
{
    // TODO: This belongs in a proper allocator. 
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = 1;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ASSERT_SUCCESS(g_GraphicDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_RTVDescriptorHeap)));

    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ASSERT_SUCCESS(g_GraphicDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_SRVDescriptorHeap)));
}

void TextureResource::CreateViews()
{
    D3D12_RENDER_TARGET_VIEW_DESC RTVDesc = {};
    RTVDesc.Format = m_Format;
    RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    RTVDesc.Texture2D.MipSlice = 0;

    D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.Format = m_Format;
    SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    SRVDesc.Texture2D.MipLevels = m_NumMips;
    SRVDesc.Texture2D.MostDetailedMip = 0;

    g_GraphicDevice->CreateRenderTargetView(m_Resource.Get(), &RTVDesc, m_RTVHandle);
    g_GraphicDevice->CreateShaderResourceView(m_Resource.Get(), &SRVDesc, m_SRVHandle);
}

ETH_NAMESPACE_END

