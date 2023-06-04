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

#pragma once

#include "graphics/pch.h"
#include "graphics/memory/uploadbufferallocator.h"
#include "graphics/rhi/rhicommandlist.h"
#include "graphics/rhi/rhipipelinestate.h"

namespace Ether::Graphics
{
class RhiCommandQueue;
class RhiCommandAllocator;
class CommandAllocatorPool;

class ETH_GRAPHIC_DLL CommandContext : public NonCopyable, public NonMovable
{
public:
    CommandContext(RhiCommandType type, const std::string& contextName = "Unnamed Command Context");
    ~CommandContext();

    inline RhiCommandList& GetCommandList() const { return *m_CommandList; }
    inline RhiCommandQueue& GetCommandQueue() const { return *m_CommandQueue; }

public:
    void SetMarker(const std::string& name);
    void PushMarker(const std::string& name);
    void PopMarker();

    void TransitionResource(RhiResource& resource, RhiResourceState newState);
    void SetDescriptorHeap(const RhiDescriptorHeap& descriptorHeap);
    void SetPipelineState(const RhiPipelineState& pipelineState);

    void SetRootConstantBuffer(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr);

    void CopyResource(RhiResource& src, RhiResource& dest);
    void CopyBufferRegion(RhiResource& src, RhiResource& dest, size_t size, size_t srcOffset = 0, size_t destOffset = 0);
    void InitializeBufferRegion(RhiResource& dest, const void* data, size_t size, size_t destOffset = 0);

    void InsertUavBarrier(const RhiResource& uavResource);
    void BuildTopLevelAccelerationStructure(RhiAccelerationStructure& accelStructure);
    void BuildBottomLevelAccelerationStructure(RhiAccelerationStructure& accelStructure);
    void SetRaytracingPipelineState(const RhiRaytracingPipelineState& pipelineState);
    void SetComputeRootSignature(const RhiRootSignature& rootSignature);

    void FinalizeAndExecute(bool waitForCompletion = false);
    void Reset();

protected:
    std::string m_Name;
    RhiCommandType m_Type;

    RhiCommandQueue* m_CommandQueue;
    RhiCommandAllocator* m_CommandAllocator;
    CommandAllocatorPool* m_CommandAllocatorPool;

    std::unique_ptr<RhiCommandList> m_CommandList;
    std::unique_ptr<UploadBufferAllocator> m_UploadBufferAllocator;
};
} // namespace Ether::Graphics
