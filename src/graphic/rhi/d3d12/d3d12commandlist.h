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

#include "graphic/rhi/rhicommandlist.h"

ETH_NAMESPACE_BEGIN

class D3D12CommandList : public RHICommandList
{
public:
    D3D12CommandList(RHICommandListType type) : RHICommandList(type) {}
    ~D3D12CommandList() override = default;

public:
    RHIResult SetViewport(const RHIViewportDesc& viewport) override;
    RHIResult SetScissor(const RHIScissorDesc& scissor) override;
    RHIResult SetPrimitiveTopology(RHIPrimitiveTopology primitiveTopology) override;
    RHIResult SetPipelineState(const RHIPipelineStateHandle pipelineState) override;
    RHIResult SetGraphicRootSignature(const RHIRootSignatureHandle rootSignature) override;
    RHIResult SetVertexBuffer(const RHIVertexBufferViewDesc& vertexBuffer) override;
    RHIResult SetIndexBuffer(const RHIIndexBufferViewDesc& indexBuffer) override;
    RHIResult SetRenderTargets(const RHISetRenderTargetsDesc& desc) override;
    RHIResult SetRootConstants(const RHISetRootConstantsDesc& desc) override;
    RHIResult SetDescriptorHeaps(const RHISetDescriptorHeapsDesc& desc) override;

    RHIResult ClearRenderTargetView(const RHIClearRenderTargetViewDesc& desc) override;
    RHIResult ClearDepthStencilView(const RHIClearDepthStencilViewDesc& desc) override;

    RHIResult CopyBufferRegion(const RHICopyBufferRegionDesc& desc) override;
    RHIResult TransitionResource(const RHIResourceTransitionDesc& desc) override;
    RHIResult DrawIndexedInstanced(const RHIDrawIndexedInstancedDesc& desc) override;

    RHIResult Reset(const RHICommandAllocatorHandle commandAllocator) override;
    RHIResult Close() override;

private:
    friend class D3D12Device;
    friend class D3D12CommandQueue;

    friend class GuiRenderer;

    wrl::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
};

ETH_NAMESPACE_END
