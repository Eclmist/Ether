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
    CommandContext(const char* contextName, RhiCommandType type = RhiCommandType::Graphic);
    ~CommandContext() = default;

public:
    inline RhiCommandList& GetCommandList() const { return *m_CommandList; }
    inline UploadBufferAllocator& GetUploadBufferAllocator() const { return *m_UploadBufferAllocator; }

public:
    void Reset();
    void FinalizeAndExecute(bool waitForCompletion = false);

    // Markers
    void SetMarker(const std::string& name);
    void PushMarker(const std::string& name);
    void PopMarker();

    // Common
    void SetDescriptorHeap(const RhiDescriptorHeap& descriptorHeap);
    void SetPipelineState(const RhiPipelineState& pipelineState);

    // Shader Data
    void SetComputeRootSignature(const RhiRootSignature& rootSignature);
    void SetComputeRootConstant(uint32_t rootParameterIndex, uint32_t data, uint32_t destOffset);
    void SetComputeRootConstantBufferView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr);
    void SetComputeRootShaderResourceView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr);
    void SetComputeRootUnorderedAccessView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr);
    void SetComputeRootDescriptorTable(uint32_t rootParameterIndex, RhiGpuAddress baseAddress);

    // Raytracing
    void BuildTopLevelAccelerationStructure(const RhiAccelerationStructure& accelStructure);
    void BuildBottomLevelAccelerationStructure(const RhiAccelerationStructure& accelStructure);
    void SetRaytracingShaderBindingTable(const RhiResource* bindTable);
    void SetRaytracingPipelineState(const RhiRaytracingPipelineState& pipelineState);

    // Barriers
    void InsertUavBarrier(const RhiResource& uavResource);
    void TransitionResource(RhiResource& resource, RhiResourceState newState);

    // Dispatches
    void InitializeBufferRegion(RhiResource& dest, const void* data, uint32_t size, uint32_t destOffset = 0);
    void InitializeTexture(RhiResource& dest, void** data, uint32_t numMips, uint32_t width, uint32_t height, uint32_t bytesPerPixel);
    void CopyResource(RhiResource& src, RhiResource& dest);
    void CopyBufferRegion(RhiResource& src, RhiResource& dest, uint32_t size, uint32_t srcOffset = 0, uint32_t destOffset = 0);
    void DispatchRays(uint32_t x, uint32_t y, uint32_t z);

protected:
    const char* m_Name;
    RhiCommandType m_Type;

    std::unique_ptr<RhiCommandList> m_CommandList;
    std::unique_ptr<UploadBufferAllocator> m_UploadBufferAllocator;

    // Raytracing
    const RhiResource* m_RaytracingBindTable;
};
} // namespace Ether::Graphics
