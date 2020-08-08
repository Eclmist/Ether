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

#include "graphic/gfxtimer.h"
#include "imgui/imguimanager.h"

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
    void ToggleImGui();
    void Resize(uint32_t width, uint32_t height);

private:
    void ResetCommandList(); // This should be done in a worker thread
    void ClearRenderTarget(); // This should be done in a worker thread
    void RenderKMS();
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

    // TODO: What is the best way to store multiple command queues?
    std::unique_ptr<DX12CommandAllocator> m_CommandAllocators[ETH_NUM_SWAPCHAIN_BUFFERS];
    std::unique_ptr<DX12CommandList> m_CommandList;

    std::unique_ptr<DX12CommandQueue> m_DirectCommandQueue;
    std::unique_ptr<DX12CommandQueue> m_ComputeCommandQueue;
    std::unique_ptr<DX12CommandQueue> m_CopyCommandQueue;

    std::unique_ptr<DX12DescriptorHeap> m_RTVDescriptorHeap;
    std::unique_ptr<DX12DescriptorHeap> m_SRVDescriptorHeap;
    uint32_t m_RTVDescriptorSize;

    // Synchronization objects
    std::unique_ptr<DX12Fence> m_Fence;
    uint64_t m_FenceValues[ETH_NUM_SWAPCHAIN_BUFFERS];

private:
    GfxTimer m_Timer;



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

    wrl::ComPtr<ID3D12RootSignature> m_RootSignature;
    wrl::ComPtr<ID3D12Resource> m_VertexBuffer;
    wrl::ComPtr<ID3D12Resource> m_IntermediateVertexBuffer; // to be used to upload vertex buffer for the first time

    wrl::ComPtr<ID3D12Resource> m_IndexBuffer;
    wrl::ComPtr<ID3D12Resource> m_IntermediateIndexBuffer; // to be used to upload index buffer for the first time

    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

    std::unique_ptr<DX12PipelineState> m_PipelineState;

    D3D12_VIEWPORT m_Viewport;
    D3D12_RECT m_ScissorRect;


    std::unique_ptr<DX12CommandAllocator> m_CopyCommandAllocators[ETH_NUM_SWAPCHAIN_BUFFERS];
    std::unique_ptr<DX12CommandList> m_CopyCommandList;

};
