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

#include "graphic/hal/dx12swapchain.h"

void DX12SwapChain::CreateSwapChain()
{
    if (m_SwapChain != nullptr)
        return;

    wrl::ComPtr<IDXGIFactory4> dxgiFactory4 = CreateDxgiFactory();
    wrl::ComPtr<IDXGISwapChain1> swapChain1;
    DXGI_SWAP_CHAIN_DESC1 desc = BuildSwapChainDescriptor();

    ThrowIfFailed(dxgiFactory4->CreateSwapChainForHwnd(m_CommandQueue.Get(), m_hWnd, &desc, nullptr, nullptr, &swapChain1));
    ThrowIfFailed(dxgiFactory4->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER));
    ThrowIfFailed(swapChain1.As(&m_SwapChain));
}

void DX12SwapChain::UpdateRenderTargets()
{
    auto rtvSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0; i < m_NumBuffers; ++i)
    {
        ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_Buffers[i])));
        m_Device->CreateRenderTargetView(m_Buffers[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(rtvSize);
    }
}

DXGI_SWAP_CHAIN_DESC1 DX12SwapChain::BuildSwapChainDescriptor()
{
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = m_FrameWidth;
    swapChainDesc.Height = m_FrameHeight;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = m_NumBuffers;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = 0;

    return swapChainDesc;
}
