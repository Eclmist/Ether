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

#include "graphic/rhi/rhidevice.h"

ETH_NAMESPACE_BEGIN

class D3D12Device : public RhiDevice
{
public:
    D3D12Device() = default;
    ~D3D12Device() override = default;

public:
    RhiResult CreateCommandAllocator(const RhiCommandAllocatorDesc& desc, RhiCommandAllocatorHandle& allocator) const override;
    RhiResult CreateCommandList(const RhiCommandListDesc& desc, RhiCommandListHandle& cmdList) const override;
    RhiResult CreateCommandQueue(const RhiCommandQueueDesc& desc, RhiCommandQueueHandle& cmdQueue) const override;
    RhiResult CreateDescriptorHeap(const RhiDescriptorHeapDesc& desc, RhiDescriptorHeapHandle& descriptorHeap) const override;
    RhiResult CreateFence(RhiFenceHandle& fence) const override;
    RhiResult CreatePipelineState(const RhiPipelineStateDesc& desc, RhiPipelineStateHandle& pipelineState) const override;
    RhiResult CreateRootParameter(RhiRootParameterHandle& rootParameter) const override;
    RhiResult CreateRootSignature(const RhiRootSignatureDesc& desc, RhiRootSignatureHandle& rootSignature) const override;
    RhiResult CreateSwapChain(const RhiSwapChainDesc& desc, RhiSwapChainHandle& swapChain) const override;

    RhiResult CreateRenderTargetView(const RhiRenderTargetViewDesc& desc, RhiRenderTargetViewHandle& rtvHandle) const override;
    RhiResult CreateDepthStencilView(const RhiDepthStencilViewDesc& desc, RhiDepthStencilViewHandle& dsvHandle) const override;
    RhiResult CreateShaderResourceView(const RhiShaderResourceViewDesc& desc, RhiShaderResourceViewHandle& srvHandle) const override;
    RhiResult CreateConstantBufferView(const RhiConstantBufferViewDesc& desc, RhiConstantBufferViewHandle& cbvHandle) const override;
    RhiResult CreateUnorderedAccessView(const RhiUnorderedAccessViewDesc& desc, RhiUnorderedAccessViewHandle& uavHandle) const override;

    RhiResult CreateCommittedResource(const RhiCommitedResourceDesc& desc, RhiResourceHandle& resourceHandle) const override;

protected:
    friend class D3D12Module;
    // TODO: Remove after GUI-Rhi refactor
    friend class GuiRenderer;
    wrl::ComPtr<ID3D12Device3> m_Device;
};

ETH_NAMESPACE_END

