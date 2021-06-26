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
#include "graphic/hal/dx12commandlist.h"
#include "graphic/hal/dx12descriptorheap.h"

ETH_NAMESPACE_BEGIN

class DX12SwapChain : public DX12Component<IDXGISwapChain4>
{
public:
    DX12SwapChain(
        HWND hWnd,
        wrl::ComPtr<ID3D12Device3> device,
        wrl::ComPtr<ID3D12CommandQueue> commandQueue,
        uint32_t frameWidth,
        uint32_t frameHeight);

    void CreateRenderTargetViews(wrl::ComPtr<ID3D12Device3> device, wrl::ComPtr<ID3D12DescriptorHeap> descriptorHeap);
    void UpdateBackBufferIndex();
    void ResizeBuffers(uint32_t width, uint32_t height);
    DXGI_SWAP_CHAIN_DESC1 BuildSwapChainDescriptor() const;

public:
    inline wrl::ComPtr<IDXGISwapChain4> Get() override { return m_SwapChain; };
    inline wrl::ComPtr<ID3D12Resource> GetCurrentBackBuffer() const { return m_Buffers[m_CurrentBackBufferIndex]; };
    inline uint32_t GetCurrentBackBufferIndex() const { return m_CurrentBackBufferIndex; };

private:
    wrl::ComPtr<IDXGISwapChain4> m_SwapChain;
    wrl::ComPtr<ID3D12Resource> m_Buffers[ETH_NUM_SWAPCHAIN_BUFFERS];

    uint32_t m_FrameWidth;
    uint32_t m_FrameHeight;
    uint32_t m_CurrentBackBufferIndex;
};

ETH_NAMESPACE_END
