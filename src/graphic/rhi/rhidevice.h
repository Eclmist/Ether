/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

ETH_NAMESPACE_BEGIN

class RhiDevice
{
public:
    RhiDevice() = default;
    virtual ~RhiDevice() = default;

public:
    virtual RhiResult CreateCommandAllocator(const RhiCommandAllocatorDesc& desc, RhiCommandAllocatorHandle& allocator) const = 0;
    virtual RhiResult CreateCommandList(const RhiCommandListDesc& desc, RhiCommandListHandle& cmdList) const = 0;
    virtual RhiResult CreateCommandQueue(const RhiCommandQueueDesc& desc, RhiCommandQueueHandle& cmdQueue) const = 0;
    virtual RhiResult CreateDescriptorHeap(const RhiDescriptorHeapDesc& desc, RhiDescriptorHeapHandle& descriptorHeap) const = 0;
    virtual RhiResult CreateFence(RhiFenceHandle& fence) const = 0;
    virtual RhiResult CreatePipelineState(const RhiPipelineStateDesc& desc, RhiPipelineStateHandle& pipelineState) const = 0;
    virtual RhiResult CreateRootParameter(RhiRootParameterHandle& rootParameter) const = 0;
    virtual RhiResult CreateRootSignature(const RhiRootSignatureDesc& desc, RhiRootSignatureHandle& rootSignature) const = 0;
    virtual RhiResult CreateSwapChain(const RhiSwapChainDesc& desc, RhiSwapChainHandle& swapChain) const = 0;

    virtual RhiResult CreateRenderTargetView(const RhiRenderTargetViewDesc& desc, RhiRenderTargetViewHandle& rtvHandle) const = 0;
    virtual RhiResult CreateDepthStencilView(const RhiDepthStencilViewDesc& desc, RhiDepthStencilViewHandle& dsvHandle) const = 0;
    virtual RhiResult CreateShaderResourceView(const RhiShaderResourceViewDesc& desc, RhiShaderResourceViewHandle& srvHandle) const = 0;
    virtual RhiResult CreateConstantBufferView(const RhiConstantBufferViewDesc& desc, RhiConstantBufferViewHandle& cbvHandle) const = 0;
    virtual RhiResult CreateUnorderedAccessView(const RhiUnorderedAccessViewDesc& desc, RhiUnorderedAccessViewHandle& uavHandle) const = 0;

    virtual RhiResult CreateCommittedResource(const RhiCommitedResourceDesc& desc, RhiResourceHandle& resourceHandle) const = 0;
};

ETH_NAMESPACE_END

