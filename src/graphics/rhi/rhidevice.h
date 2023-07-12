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
#include "graphics/rhi/rhiaccelerationstructure.h"
#include "graphics/rhi/rhirootsignature.h"
#include "graphics/rhi/rhigraphicpipelinestate.h"
#include "graphics/rhi/rhicomputepipelinestate.h"
#include "graphics/rhi/rhishader.h"

namespace Ether::Graphics
{
class RhiDevice : public NonCopyable, public NonMovable
{
public:
    RhiDevice() = default;
    virtual ~RhiDevice() = default;

public:
    virtual std::unique_ptr<RhiCommandAllocator> CreateCommandAllocator(const RhiCommandType& type) const = 0;
    virtual std::unique_ptr<RhiCommandList> CreateCommandList(const char* name, const RhiCommandType& type) const = 0;
    virtual std::unique_ptr<RhiCommandQueue> CreateCommandQueue(const RhiCommandType& type) const = 0;
    virtual std::unique_ptr<RhiDescriptorHeap> CreateDescriptorHeap(const RhiDescriptorHeapType& type, uint32_t numDescriptors, bool isShaderVisible) const = 0;
    virtual std::unique_ptr<RhiFence> CreateFence() const = 0;
    virtual std::unique_ptr<RhiSwapChain> CreateSwapChain(const RhiSwapChainDesc& desc) const = 0;
    virtual std::unique_ptr<RhiShader> CreateShader(const RhiShaderDesc& desc) const = 0;
    virtual std::unique_ptr<RhiRootSignatureDesc> CreateRootSignatureDesc(uint32_t numParams, uint32_t numSamplers, bool isLocal = false) const = 0;
    virtual std::unique_ptr<RhiGraphicPipelineStateDesc> CreateGraphicPipelineStateDesc() const = 0;
    virtual std::unique_ptr<RhiComputePipelineStateDesc> CreateComputePipelineStateDesc() const = 0;
    virtual std::unique_ptr<RhiRaytracingPipelineStateDesc> CreateRaytracingPipelineStateDesc() const = 0;

    virtual std::unique_ptr<RhiResource> CreateRaytracingShaderBindingTable(const char* name, const RhiRaytracingShaderBindingTableDesc& desc) const = 0;
    virtual std::unique_ptr<RhiAccelerationStructure> CreateAccelerationStructure(const RhiTopLevelAccelerationStructureDesc& desc) const = 0;
    virtual std::unique_ptr<RhiAccelerationStructure> CreateAccelerationStructure(const RhiBottomLevelAccelerationStructureDesc& desc) const = 0;

    virtual std::unique_ptr<RhiResource> CreateCommittedResource(const RhiCommitedResourceDesc& desc) const = 0;

public:
    virtual void CopyDescriptors(uint32_t numDescriptors, RhiCpuAddress srcAddr, RhiCpuAddress destAddr, RhiDescriptorHeapType type) const = 0;
    virtual void CopySampler(const RhiSamplerParameterDesc& sampler, RhiCpuAddress destAddr) const = 0;

    virtual void InitializeRenderTargetView(RhiRenderTargetView& rtv, const RhiResource& resource) const = 0;
    virtual void InitializeDepthStencilView(RhiDepthStencilView& dsv, const RhiResource& resource) const = 0;
    virtual void InitializeShaderResourceView(RhiShaderResourceView& srv, const RhiResource& resource) const = 0;
    virtual void InitializeConstantBufferView(RhiConstantBufferView& cbv, const RhiResource& resource) const = 0;
    virtual void InitializeUnorderedAccessView(RhiUnorderedAccessView& uav, const RhiResource& resource) const = 0;

protected:
    friend class RhiRootSignatureDesc;
    friend class RhiGraphicPipelineStateDesc;
    friend class RhiComputePipelineStateDesc;
    friend class RhiRaytracingPipelineStateDesc;

    virtual std::unique_ptr<RhiPipelineState> CreateGraphicPipelineState(const char* name, const RhiGraphicPipelineStateDesc& desc) const = 0;
    virtual std::unique_ptr<RhiPipelineState> CreateComputePipelineState(const char* name, const RhiComputePipelineStateDesc& desc) const = 0;
    virtual std::unique_ptr<RhiPipelineState> CreateRaytracingPipelineState(const char* name, const RhiRaytracingPipelineStateDesc& desc) const = 0;
    virtual std::unique_ptr<RhiRootSignature> CreateRootSignature(const char* name, const RhiRootSignatureDesc& desc) const = 0;
};
} // namespace Ether::Graphics
