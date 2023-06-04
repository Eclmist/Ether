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
#include "graphics/rhi/rhidevice.h"
#include "graphics/rhi/dx12/dx12includes.h"

namespace Ether::Graphics
{
class Dx12Device : public RhiDevice
{
public:
    Dx12Device() = default;
    ~Dx12Device() override = default;

public:
    std::unique_ptr<RhiCommandAllocator> CreateCommandAllocator(const RhiCommandAllocatorDesc& desc) const override;
    std::unique_ptr<RhiCommandList> CreateCommandList(const RhiCommandListDesc& desc) const override;
    std::unique_ptr<RhiCommandQueue> CreateCommandQueue(const RhiCommandQueueDesc& desc) const override;
    std::unique_ptr<RhiDescriptorHeap> CreateDescriptorHeap(const RhiDescriptorHeapDesc& desc) const override;
    std::unique_ptr<RhiFence> CreateFence() const override;
    std::unique_ptr<RhiSwapChain> CreateSwapChain(const RhiSwapChainDesc& desc) const override;
    std::unique_ptr<RhiShader> CreateShader(const RhiShaderDesc& desc) const override;
    std::unique_ptr<RhiRootSignatureDesc> CreateRootSignatureDesc(uint32_t numParams, uint32_t numSamplers, bool isLocal) const override;
    std::unique_ptr<RhiPipelineStateDesc> CreatePipelineStateDesc() const override;
    std::unique_ptr<RhiRootSignature> CreateRootSignature(const RhiRootSignatureDesc& desc) const override;
    std::unique_ptr<RhiPipelineState> CreatePipelineState(const RhiPipelineStateDesc& desc) const override;

    std::unique_ptr<RhiRenderTargetView> CreateRenderTargetView(const RhiRenderTargetViewDesc& desc) const override;
    std::unique_ptr<RhiDepthStencilView> CreateDepthStencilView(const RhiDepthStencilViewDesc& desc) const override;
    std::unique_ptr<RhiShaderResourceView> CreateShaderResourceView(const RhiShaderResourceViewDesc& desc) const override;
    std::unique_ptr<RhiConstantBufferView> CreateConstantBufferView(const RhiConstantBufferViewDesc& desc) const override;
    std::unique_ptr<RhiUnorderedAccessView> CreateUnorderedAccessView(const RhiUnorderedAccessViewDesc& desc) const override;

    std::unique_ptr<RhiAccelerationStructure> CreateAccelerationStructure(const RhiTopLevelAccelerationStructureDesc& desc) const override;
    std::unique_ptr<RhiAccelerationStructure> CreateAccelerationStructure(const RhiBottomLevelAccelerationStructureDesc& desc) const override;
    std::unique_ptr<RhiRaytracingPipelineState> CreateRaytracingPipelineState(const RhiRaytracingPipelineStateDesc& desc) const override;
    std::unique_ptr<RhiRaytracingShaderBindingTable> CreateRaytracingShaderBindingTable(const RhiRaytracingShaderBindingTableDesc& desc) const override;

    std::unique_ptr<RhiResource> CreateCommittedResource(const RhiCommitedResourceDesc& desc) const override;

protected:
    friend class Dx12Module;
    friend class Dx12ImguiWrapper;

    wrl::ComPtr<ID3D12Device5> m_Device;
};
} // namespace Ether::Graphics
