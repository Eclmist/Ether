/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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
#include "graphics/rhi/rhicommandlist.h"
#include "graphics/rhi/rhicomputepipelinestate.h"
#include "graphics/rhi/rhigraphicpipelinestate.h"
#include "graphics/rhi/rhiraytracingpipelinestate.h"
#include "graphics/rhi/rhirootsignaturebindingtable.h"
#include "graphics/memory/uploadbufferallocator.h"
#include "graphics/schedule/frameschedulerutils.h"

namespace Ether::Graphics
{
class RhiCommandQueue;
class RhiCommandAllocator;
class CommandAllocatorPool;
class ResourceContext;

class ETH_GRAPHIC_DLL CommandContext : public NonCopyable, public NonMovable
{
public:
    CommandContext(
        const char* contextName,
        RhiCommandType type = RhiCommandType::Graphic,
        size_t uploadBufferSize = _4MiB);
    virtual ~CommandContext() = default;

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
    void SetSrvCbvUavDescriptorHeap(const RhiDescriptorHeap& descriptorHeap);
    void SetSamplerDescriptorHeap(const RhiDescriptorHeap& descriptorHeap);
    void SetGraphicPipelineState(const RhiGraphicPipelineState& pipelineState);
    void SetComputePipelineState(const RhiComputePipelineState& pipelineState);
    void SetRaytracingPipelineState(const RhiRaytracingPipelineState& pipelineState);
    void SetResourceContext(const ResourceContext& resourceContext);

    // Shader Data
    void Bind(const GFX_STATIC::StaticResourceWrapper<RhiConstantBufferView>& wrapper, uint64_t offset = 0);
    void Bind(const GFX_STATIC::StaticResourceWrapper<RhiShaderResourceView>& wrapper, uint64_t offset = 0);
    void Bind(const GFX_STATIC::StaticResourceWrapper<RhiUnorderedAccessView>& wrapper, uint64_t offset = 0);
    void Bind(const GFX_STATIC::StaticResourceWrapper<RhiAccelerationStructureResourceView>& wrapper, uint64_t offset = 0);
    void Bind(const std::string& name, const GFX_STATIC::StaticResourceWrapper<RhiConstantBufferView>& wrapper, uint64_t offset = 0);
    void Bind(const std::string& name, const GFX_STATIC::StaticResourceWrapper<RhiShaderResourceView>& wrapper, uint64_t offset = 0);
    void Bind(const std::string& name, const GFX_STATIC::StaticResourceWrapper<RhiUnorderedAccessView>& wrapper, uint64_t offset = 0);
    void Bind(const std::string& name, const GFX_STATIC::StaticResourceWrapper<RhiAccelerationStructureResourceView>& wrapper, uint64_t offset = 0);
    void Bind(const std::string& name, RhiShaderVisibleResourceView* resource, uint64_t offset = 0);
    void Bind(const std::string& name, RhiGpuAddress address, uint64_t offset = 0);
    void Bind(const std::string& name, uint32_t value, uint64_t offset = 0);

    // TODO: Deprecate
    void SetComputeRootSignature(const RhiRootSignature& rootSignature);
    void SetComputeRootConstant(uint32_t rootParameterIndex, uint32_t data, uint32_t destOffset);
    void SetComputeRootConstantBufferView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr);
    void SetComputeRootShaderResourceView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr);
    void SetComputeRootUnorderedAccessView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr);
    void SetComputeRootDescriptorTable(uint32_t rootParameterIndex, RhiGpuAddress baseAddress);

    // Raytracing
    void BuildTopLevelAccelerationStructure(const RhiAccelerationStructure& accelStructure);
    void BuildBottomLevelAccelerationStructure(const RhiAccelerationStructure& accelStructure);
    void RefitBottomLevelAccelerationStructure(const RhiAccelerationStructure& accelStructure);
    void SetRaytracingShaderBindingTable(const RhiResource* bindTable);

    // Barriers
    void InsertUavBarrier(const RhiResource& uavResource);
    void TransitionResource(RhiResource& resource, RhiResourceState newState);

    // Dispatches
    void InitializeBufferRegion(RhiResource& dest, const void* data, uint32_t size, uint32_t destOffset = 0);
    void InitializeTexture(RhiResource& dest, void** data, uint32_t numMips, uint32_t width, uint32_t height, uint32_t bytesPerPixel);
    void CopyResource(RhiResource& src, RhiResource& dest);
    void CopyBufferRegion(RhiResource& src, RhiResource& dest, uint32_t size, uint32_t srcOffset = 0, uint32_t destOffset = 0);
    void CopyTextureToBuffer(RhiResource& src, RhiResource& dest, uint32_t width, uint32_t height);
    void Dispatch(uint32_t x, uint32_t y, uint32_t z);
    void DispatchRays(uint32_t x, uint32_t y, uint32_t z);

protected:
    const char* m_Name;
    RhiCommandType m_Type;

    std::unique_ptr<RhiCommandList> m_CommandList;
    std::unique_ptr<UploadBufferAllocator> m_UploadBufferAllocator;
    std::unique_ptr<RhiRootSignatureBindingTable> m_RootSignatureBindingTable;

    const RhiDescriptorHeap* m_SrvCbvUavHeap;
    const RhiDescriptorHeap* m_SamplerHeap;

    const RhiResource* m_RaytracingBindTable;
};
} // namespace Ether::Graphics

