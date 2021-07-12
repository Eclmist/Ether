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

#include "graphic/resource/gpuresource.h"
#include "graphic/resource/bufferresource.h"
#include "graphic/resource/textureresource.h"
#include "graphic/resource/depthstencilresource.h"
#include "graphic/resource/virtualbuffer/linearallocator.h"

ETH_NAMESPACE_BEGIN

class CommandQueue;

class GraphicContext : NonCopyable
{
public:
    void Initialize(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);
    void Shutdown();
    void Reset();

    CommandQueue& GetCommandQueue() const;
    uint64_t GetCompletionFenceValue() const;

public:
    inline ID3D12GraphicsCommandList* GetCommandList() const { return m_CommandList.Get(); }

    inline ethXMMatrix GetViewMatrix() const { return m_ViewMatrix; }
    inline ethXMMatrix GetProjectionMatrix() const { return m_ProjectionMatrix; }

    inline void SetViewMatrix(ethXMMatrix viewMatrix) { m_ViewMatrix = viewMatrix; }
    inline void SetProjectionMatrix(ethXMMatrix projectionMatrix) { m_ProjectionMatrix = projectionMatrix; }

public:
    void ClearColor(TextureResource& texture, ethVector4 color);
    void ClearDepth(DepthStencilResource& depthTex, float val);
    void TransitionResource(GpuResource& resource, D3D12_RESOURCE_STATES newState);
    void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE rtv);
    void FinalizeAndExecute(bool waitForCompletion = false);

public:
    static void InitializeBuffer(BufferResource& dest, const void* data, size_t size, size_t dstOffset = 0);

private:
    // One descriptor heap is certainly not enough for actual rendering. This is why
    // most D3D12 apps create some kind of allocator to get just enough descriptor heaps.
    // TODO: Handle descriptor heap allocation once we're doing more than just clearing
    // the screen.
    ID3D12DescriptorHeap* m_CurrentDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    wrl::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
    ID3D12CommandAllocator* m_CommandAllocator;
    D3D12_COMMAND_LIST_TYPE m_Type;

private:
    LinearAllocator m_GpuAllocator;

private:
    ethXMMatrix m_ViewMatrix;
    ethXMMatrix m_ProjectionMatrix;
};

ETH_NAMESPACE_END

