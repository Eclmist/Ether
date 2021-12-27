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

ETH_NAMESPACE_BEGIN

class RHIDevice
{
public:
    RHIDevice() = default;
	virtual ~RHIDevice() = default;

public:
    virtual RHIResult CreateCommandAllocator(const RHICommandAllocatorDesc& desc, RHICommandAllocatorHandle& allocator) const = 0;
    virtual RHIResult CreateCommandList(const RHICommandListDesc& desc, RHICommandListHandle& cmdList) const = 0;
    virtual RHIResult CreateCommandQueue(const RHICommandQueueDesc& desc, RHICommandQueueHandle& cmdQueue) const = 0;
    virtual RHIResult CreateDescriptorHeap(const RHIDescriptorHeapDesc& desc, RHIDescriptorHeapHandle& descriptorHeap) const = 0;
    virtual RHIResult CreateFence(RHIFenceHandle& fence) const = 0;
    virtual RHIResult CreatePipelineState(const RHIPipelineStateDesc& desc, RHIPipelineStateHandle& pipelineState) const = 0;
    virtual RHIResult CreateRootSignature(const RHIRootSignatureDesc& desc, RHIRootSignatureHandle& rootSignature) const = 0;
    virtual RHIResult CreateSwapChain(const RHISwapChainDesc& desc, RHISwapChainHandle& swapChain) const = 0;

    virtual RHIResult CreateRenderTargetView(const RHIRenderTargetViewDesc& desc, RHIRenderTargetViewHandle& rtvHandle) const = 0;
    virtual RHIResult CreateDepthStencilView(const RHIDepthStencilViewDesc& desc, RHIDepthStencilViewHandle& dsvHandle) const = 0;
    virtual RHIResult CreateShaderResourceView(const RHIShaderResourceViewDesc& desc, RHIShaderResourceViewHandle& srvHandle) const = 0;
    virtual RHIResult CreateConstantBufferView(const RHIConstantBufferViewDesc& desc, RHIConstantBufferViewHandle& cbvHandle) const = 0;
    virtual RHIResult CreateUnorderedAccessView(const RHIUnorderedAccessViewDesc& desc, RHIUnorderedAccessViewHandle& uavHandle) const = 0;

    virtual RHIResult CreateCommittedResource(const RHICommitedResourceDesc& desc, RHIResourceHandle& resourceHandle) const = 0;
};

ETH_NAMESPACE_END

