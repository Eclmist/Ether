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

#include "graphics/rhi/dx12/dx12swapchain.h"
#include "graphics/rhi/dx12/dx12resource.h"
#include "graphics/rhi/dx12/dx12translation.h"

#ifdef ETH_GRAPHICS_DX12

Ether::Graphics::Dx12SwapChain::Dx12SwapChain()
{
    ResetBuffers();
}

uint32_t Ether::Graphics::Dx12SwapChain::GetCurrentBackBufferIndex() const
{
    return m_SwapChain->GetCurrentBackBufferIndex();
}

Ether::Graphics::RhiResource& Ether::Graphics::Dx12SwapChain::GetBuffer(uint8_t index) const
{
    Dx12Resource* dx12Resource = dynamic_cast<Dx12Resource*>(m_BufferResources[index].get());

    if (dx12Resource->m_Resource != nullptr)
        return *m_BufferResources[index];

    m_SwapChain->GetBuffer(index, IID_PPV_ARGS(&dx12Resource->m_Resource));
    dx12Resource->m_Resource->SetName((L"Swapchain Buffer " + std::to_wstring(index)).c_str());
    return *m_BufferResources[index];
}

void Ether::Graphics::Dx12SwapChain::ResizeBuffers(const ethVector2u& size)
{
    ResetBuffers();

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    HRESULT hr = m_SwapChain->GetDesc(&swapChainDesc);
    if (FAILED(hr))
    {
        LogGraphicsError("Failed to retrieve current swapchain info for resizing");
        return;
    }

    hr = m_SwapChain->ResizeBuffers(
        swapChainDesc.BufferCount,
        size.x,
        size.y,
        swapChainDesc.BufferDesc.Format,
        swapChainDesc.Flags);

    if (FAILED(hr))
        LogGraphicsError("Failed to resize swapchain buffers");
}

void Ether::Graphics::Dx12SwapChain::ResetBuffers()
{
    for (int i = 0; i < MaxSwapChainBuffers; ++i)
        m_BufferResources[i] = std::make_unique<Dx12Resource>(("Swapchain RenderTarget" + std::to_string(i)).c_str());
}

#include "graphics/graphiccore.h"

void Ether::Graphics::Dx12SwapChain::Present(uint8_t numVblanks)
{
    HRESULT hr = m_SwapChain->Present(numVblanks, numVblanks > 0 ? 0 : DXGI_PRESENT_ALLOW_TEARING);

    if (FAILED(hr))
        LogGraphicsFatal("Failed to present swapchain buffers");
}

#endif // ETH_GRAPHICS_DX12
