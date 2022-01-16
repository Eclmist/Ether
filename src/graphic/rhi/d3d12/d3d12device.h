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

#include "graphic/rhi/rhidevice.h"

ETH_NAMESPACE_BEGIN

class D3D12Device : public RHIDevice
{
public:
    D3D12Device() = default;
    ~D3D12Device() override = default;

public:
    RHIResult CreateCommandAllocator(const RHICommandAllocatorDesc& desc, RHICommandAllocatorHandle& allocator) const override;
    RHIResult CreateCommandList(const RHICommandListDesc& desc, RHICommandListHandle& cmdList) const override;
    RHIResult CreateCommandQueue(const RHICommandQueueDesc& desc, RHICommandQueueHandle& cmdQueue) const override;
    RHIResult CreateDescriptorHeap(const RHIDescriptorHeapDesc& desc, RHIDescriptorHeapHandle& descriptorHeap) const override;
    RHIResult CreateFence(RHIFenceHandle& fence) const override;
    RHIResult CreatePipelineState(const RHIPipelineStateDesc& desc, RHIPipelineStateHandle& pipelineState) const override;
    RHIResult CreateRootParameter(RHIRootParameterHandle& rootParameter) const override;
    RHIResult CreateRootSignature(const RHIRootSignatureDesc& desc, RHIRootSignatureHandle& rootSignature) const override;
    RHIResult CreateSwapChain(const RHISwapChainDesc& desc, RHISwapChainHandle& swapChain) const override;

    RHIResult CreateRenderTargetView(const RHIRenderTargetViewDesc& desc, RHIRenderTargetViewHandle& rtvHandle) const override;
    RHIResult CreateDepthStencilView(const RHIDepthStencilViewDesc& desc, RHIDepthStencilViewHandle& dsvHandle) const override;
    RHIResult CreateShaderResourceView(const RHIShaderResourceViewDesc& desc, RHIShaderResourceViewHandle& srvHandle) const override;
    RHIResult CreateConstantBufferView(const RHIConstantBufferViewDesc& desc, RHIConstantBufferViewHandle& cbvHandle) const override;
    RHIResult CreateUnorderedAccessView(const RHIUnorderedAccessViewDesc& desc, RHIUnorderedAccessViewHandle& uavHandle) const override;

    RHIResult CreateCommittedResource(const RHICommitedResourceDesc& desc, RHIResourceHandle& resourceHandle) const override;

protected:
    friend class D3D12Module;
    // TODO: Remove after GUI-RHI refactor
    friend class GuiRenderer;
    wrl::ComPtr<ID3D12Device3> m_Device;
};

ETH_NAMESPACE_END

