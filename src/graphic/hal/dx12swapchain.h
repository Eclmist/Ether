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

#include "graphic/hal/dx12component.h"

class DX12SwapChain : public DX12Component<IDXGISwapChain4>
{
public:
    DX12SwapChain(
        wrl::ComPtr<ID3D12Device3> device,
        wrl::ComPtr<ID3D12DescriptorHeap> descriptorHeap,
        wrl::ComPtr<ID3D12CommandQueue> commandQueue,
        HWND hWnd,
        uint32_t frameWidth,
        uint32_t frameHeight)
        : m_Device(device)
        , m_DescriptorHeap(descriptorHeap)
        , m_CommandQueue(commandQueue)
        , m_hWnd(hWnd)
        , m_NumBuffers(ETH_NUM_SWAPCHAIN_BUFFERS)
        , m_FrameWidth(frameWidth)
        , m_FrameHeight(frameHeight) {};

    void CreateSwapChain();
    void UpdateRenderTargets();

public:
    inline wrl::ComPtr<IDXGISwapChain4> Get() override { return m_SwapChain; };

private:
    DXGI_SWAP_CHAIN_DESC1 BuildSwapChainDescriptor();

private:
    wrl::ComPtr<IDXGISwapChain4> m_SwapChain;
    wrl::ComPtr<ID3D12Device3> m_Device;
    wrl::ComPtr<ID3D12CommandQueue> m_CommandQueue;
    wrl::ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap;
    wrl::ComPtr<ID3D12Resource> m_Buffers[ETH_NUM_SWAPCHAIN_BUFFERS];
    wrl::ComPtr<ID3D12CommandList> m_CommandList[ETH_NUM_SWAPCHAIN_BUFFERS];

    HWND m_hWnd;
    uint8_t m_NumBuffers;
    uint32_t m_FrameWidth;
    uint32_t m_FrameHeight;

    uint32_t m_CurrentBackBufferIndex;
};
