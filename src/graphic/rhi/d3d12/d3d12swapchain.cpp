/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "d3d12swapchain.h"
#include "d3d12resource.h"
#include "d3d12translation.h"

ETH_NAMESPACE_BEGIN

uint32_t D3D12SwapChain::GetCurrentBackBufferIndex() const
{
    return m_SwapChain->GetCurrentBackBufferIndex();
}

RhiResourceHandle D3D12SwapChain::GetBuffer(uint8_t index) const
{
    D3D12Resource* d3dResource = new D3D12Resource();
    m_SwapChain->GetBuffer(index, IID_PPV_ARGS(&d3dResource->m_Resource));

    RhiResourceHandle resourceHandle;
    resourceHandle.Set(d3dResource);
    return resourceHandle;
}

RhiResult D3D12SwapChain::ResizeBuffers(RhiResizeDesc& desc)
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    ASSERT_SUCCESS(m_SwapChain->GetDesc(&swapChainDesc));
    HRESULT hr = m_SwapChain->ResizeBuffers(
        swapChainDesc.BufferCount,
        desc.m_Width,
        desc.m_Height,
        swapChainDesc.BufferDesc.Format,
        swapChainDesc.Flags
    );

    return TO_RHI_RESULT(hr);
}

RhiResult D3D12SwapChain::Present(uint8_t numVblanks)
{
    HRESULT hr = m_SwapChain->Present(numVblanks,
        numVblanks > 0 ? 0 : DXGI_PRESENT_ALLOW_TEARING);

    return TO_RHI_RESULT(hr);
}

ETH_NAMESPACE_END

