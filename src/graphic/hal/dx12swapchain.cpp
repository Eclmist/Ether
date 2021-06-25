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

#include "dx12swapchain.h"

ETH_NAMESPACE_BEGIN

DX12SwapChain::DX12SwapChain(
    HWND hWnd,
    wrl::ComPtr<ID3D12Device3> device,
    wrl::ComPtr<ID3D12CommandQueue> commandQueue,
    uint32_t frameWidth,
    uint32_t frameHeight)
    : m_FrameWidth(frameWidth)
    , m_FrameHeight(frameHeight)
    , m_CurrentBackBufferIndex(0)
{
    wrl::ComPtr<IDXGIFactory4> dxgiFactory4 = CreateDxgiFactory();
    wrl::ComPtr<IDXGISwapChain1> swapChain1;
    DXGI_SWAP_CHAIN_DESC1 desc = BuildSwapChainDescriptor();

    ThrowIfFailed(dxgiFactory4->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &desc, nullptr, nullptr, &swapChain1));
    ThrowIfFailed(dxgiFactory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
    ThrowIfFailed(swapChain1.As(&m_SwapChain));
}

void DX12SwapChain::CreateRenderTargetViews(
    wrl::ComPtr<ID3D12Device3> device,
    wrl::ComPtr<ID3D12DescriptorHeap> descriptorHeap)
{
    auto rtvSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0; i < ETH_NUM_SWAPCHAIN_BUFFERS; ++i)
    {
        ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_Buffers[i])));
        device->CreateRenderTargetView(m_Buffers[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(rtvSize);
    }
}

void DX12SwapChain::UpdateBackBufferIndex()
{
    m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

void DX12SwapChain::ResizeBuffers(uint32_t width, uint32_t height)
{
    assert((width != 0) && (height != 0) && "0 sized frame buffers are not allowed.");

    for (int i = 0; i < ETH_NUM_SWAPCHAIN_BUFFERS; ++i)
        m_Buffers[i].Reset();

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    ThrowIfFailed(m_SwapChain->GetDesc(&swapChainDesc));
    ThrowIfFailed(m_SwapChain->ResizeBuffers(
        ETH_NUM_SWAPCHAIN_BUFFERS,
        width,
        height,
        swapChainDesc.BufferDesc.Format,
        swapChainDesc.Flags));

    m_FrameWidth = width;
    m_FrameHeight = height;

    UpdateBackBufferIndex();
}

DXGI_SWAP_CHAIN_DESC1 DX12SwapChain::BuildSwapChainDescriptor() const
{
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = m_FrameWidth;
    swapChainDesc.Height = m_FrameHeight;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = ETH_NUM_SWAPCHAIN_BUFFERS;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = 0;

    return swapChainDesc;
}

ETH_NAMESPACE_END
