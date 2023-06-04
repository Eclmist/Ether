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
#include "graphics/rhi/rhipipelinestate.h"
#include "graphics/rhi/rhishader.h"

namespace Ether::Graphics
{
class RhiDevice : public NonCopyable, public NonMovable
{
public:
    RhiDevice() = default;
    virtual ~RhiDevice() = default;

public:
    virtual std::unique_ptr<RhiCommandAllocator> CreateCommandAllocator(const RhiCommandAllocatorDesc& desc) const = 0;
    virtual std::unique_ptr<RhiCommandList> CreateCommandList(const RhiCommandListDesc& desc) const = 0;
    virtual std::unique_ptr<RhiCommandQueue> CreateCommandQueue(const RhiCommandQueueDesc& desc) const = 0;
    virtual std::unique_ptr<RhiDescriptorHeap> CreateDescriptorHeap(const RhiDescriptorHeapDesc& desc) const = 0;
    virtual std::unique_ptr<RhiFence> CreateFence() const = 0;
    virtual std::unique_ptr<RhiSwapChain> CreateSwapChain(const RhiSwapChainDesc& desc) const = 0;
    virtual std::unique_ptr<RhiShader> CreateShader(const RhiShaderDesc& desc) const = 0;
    virtual std::unique_ptr<RhiRootSignatureDesc> CreateRootSignatureDesc(uint32_t numParams, uint32_t numSamplers, bool isLocal = false) const = 0;
    virtual std::unique_ptr<RhiPipelineStateDesc> CreatePipelineStateDesc() const = 0;

    virtual std::unique_ptr<RhiRenderTargetView> CreateRenderTargetView(const RhiRenderTargetViewDesc& desc) const = 0;
    virtual std::unique_ptr<RhiDepthStencilView> CreateDepthStencilView(const RhiDepthStencilViewDesc& desc) const = 0;
    virtual std::unique_ptr<RhiShaderResourceView> CreateShaderResourceView(const RhiShaderResourceViewDesc& desc) const = 0;
    virtual std::unique_ptr<RhiConstantBufferView> CreateConstantBufferView(const RhiConstantBufferViewDesc& desc) const = 0;
    virtual std::unique_ptr<RhiUnorderedAccessView> CreateUnorderedAccessView(const RhiUnorderedAccessViewDesc& desc) const = 0;

    virtual std::unique_ptr<RhiAccelerationStructure> CreateAccelerationStructure(const RhiTopLevelAccelerationStructureDesc& desc) const = 0;
    virtual std::unique_ptr<RhiAccelerationStructure> CreateAccelerationStructure(const RhiBottomLevelAccelerationStructureDesc& desc) const = 0;
    virtual std::unique_ptr<RhiRaytracingPipelineState> CreateRaytracingPipelineState(const RhiRaytracingPipelineStateDesc& desc) const = 0;
    virtual std::unique_ptr<RhiRaytracingShaderTable> CreateRaytracingShaderTable(const RhiRaytracingShaderTableDesc& desc) const = 0;

    virtual std::unique_ptr<RhiResource> CreateCommittedResource(const RhiCommitedResourceDesc& desc) const = 0;

protected:
    friend class RhiRootSignatureDesc;
    friend class RhiPipelineStateDesc;
    virtual std::unique_ptr<RhiPipelineState> CreatePipelineState(const RhiPipelineStateDesc& desc) const = 0;
    virtual std::unique_ptr<RhiRootSignature> CreateRootSignature(const RhiRootSignatureDesc& desc) const = 0;
};
} // namespace Ether::Graphics
