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
#include "engine/subsystem/enginesubsystem.h"

#include "graphic/hal/dx12includes.h"
#include "graphic/hal/dx12adapter.h"
#include "graphic/hal/dx12commandqueue.h"
#include "graphic/hal/dx12commandallocator.h"
#include "graphic/hal/dx12descriptorheap.h"
#include "graphic/hal/dx12device.h"
#include "graphic/hal/dx12pipelinestate.h"
#include "graphic/hal/dx12swapchain.h"

#include "graphic/gfx/gfxcontext.h"
#include "graphic/gfx/gfximgui.h"
#include "graphic/gfx/gfxtimer.h"
#include "graphic/gfx/gfxview.h"

class Engine;

class GfxRenderer : public EngineSubsystem
{
public:
    GfxRenderer(Engine* engine);
    ~GfxRenderer() = default;

public:
    void Initialize() override;
    void Shutdown() override;
    void Flush();
    void RenderFrame();
    void ToggleGui();
    void Resize(uint32_t width, uint32_t height);
    void SetRTCommonParams(DX12CommandList& commandList) const;

private:
    void InitRendererCore();
    void InitCommandQueues();
    void InitRTVDescriptor();
    void InitSRVDescriptor();
    void InitSwapChain();
    void InitFences();
    void InitUtilityCommandList();
    void InitGuiManager();

    void ResetUtilityCommandLists();
    void ClearRenderTarget();
    //void RenderKMS();
    void RenderGui(); 
    void Present();
    void EndOfFrame();
    void WaitForGPU();
    void EnableDebugLayer();

    DX12CommandQueue* GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const;

private:
    std::unique_ptr<DX12Adapter> m_Adapter;
    std::unique_ptr<DX12Device> m_Device;
    std::unique_ptr<DX12SwapChain> m_SwapChain;

    std::unique_ptr<DX12CommandAllocator> m_UtilityCommandAllocators[ETH_NUM_SWAPCHAIN_BUFFERS];
    std::unique_ptr<DX12CommandList> m_UtilityCommandList;

    std::unique_ptr<DX12CommandQueue> m_DirectCommandQueue;
    std::unique_ptr<DX12CommandQueue> m_ComputeCommandQueue;
    std::unique_ptr<DX12CommandQueue> m_CopyCommandQueue;

    std::unique_ptr<DX12DescriptorHeap> m_RTVDescriptorHeap;
    std::unique_ptr<DX12DescriptorHeap> m_SRVDescriptorHeap;
    uint32_t m_RTVDescriptorSize;

    std::unique_ptr<DX12Fence> m_Fence;
    uint64_t m_FenceValues[ETH_NUM_SWAPCHAIN_BUFFERS];

private:
    std::unique_ptr<GfxContext> m_Context;
    std::unique_ptr<GfxView> m_View;
    GfxImGui m_ImGui;

    // PROTOTYPE BS
public:
    
    // Used to create a ID3D12Resource that is large enough to store the vertex/index buffers
    void UpdateBufferResource(
        wrl::ComPtr<ID3D12GraphicsCommandList2> commandList,
        ID3D12Resource** pDestinationResource,
        ID3D12Resource** pIntermediateResource,
        size_t numElements, size_t elementSize, const void* bufferData,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    void LoadContent();
};
