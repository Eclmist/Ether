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

#include "system/system.h"

#include "graphic/hal/dx12includes.h"
#include "graphic/hal/dx12adapter.h"
#include "graphic/hal/dx12commandqueue.h"
#include "graphic/hal/dx12commandallocator.h"
#include "graphic/hal/dx12device.h"
#include "graphic/hal/dx12swapchain.h"
#include "graphic/hal/dx12descriptorheap.h"

#include "graphic/gfxtimer.h"

class Renderer
{
public:
    Renderer();

    void Flush();
    void Render();
    void Release();

private:
    void ResetCommandList();
    void ClearRenderTarget();
    void Present();
    void EnableDebugLayer();

private:
    std::unique_ptr<DX12Adapter> m_Adapter;
    std::unique_ptr<DX12Device> m_Device;
    std::unique_ptr<DX12SwapChain> m_SwapChain;

    // TODO: What is the best way to store multiple command queues?
    std::unique_ptr<DX12CommandAllocator> m_CommandAllocators[ETH_NUM_SWAPCHAIN_BUFFERS];
    std::unique_ptr<DX12Fence> m_Fence[ETH_NUM_SWAPCHAIN_BUFFERS];
    std::unique_ptr<DX12CommandList> m_CommandList;
    std::unique_ptr<DX12CommandQueue> m_CommandQueue;

    std::unique_ptr<DX12DescriptorHeap> m_RTVDescriptorHeap;
    uint32_t m_RTVDescriptorSize;
private:
    GfxTimer m_Timer;
};
