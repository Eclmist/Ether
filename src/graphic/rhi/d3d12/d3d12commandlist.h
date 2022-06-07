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

#include "graphic/rhi/rhicommandlist.h"

ETH_NAMESPACE_BEGIN

class D3D12CommandList : public RhiCommandList
{
public:
    D3D12CommandList(RhiCommandListType type) : RhiCommandList(type) {}
    ~D3D12CommandList() override = default;

public:
    RhiResult SetViewport(const RhiViewportDesc& viewport) override;
    RhiResult SetScissor(const RhiScissorDesc& scissor) override;
    RhiResult SetStencilRef(const RhiStencilValue& val) override;
    RhiResult SetPrimitiveTopology(RhiPrimitiveTopology primitiveTopology) override;
    RhiResult SetPipelineState(const RhiPipelineStateHandle pipelineState) override;
    RhiResult SetGraphicRootSignature(const RhiRootSignatureHandle rootSignature) override;
    RhiResult SetVertexBuffer(const RhiVertexBufferViewDesc& vertexBuffer) override;
    RhiResult SetIndexBuffer(const RhiIndexBufferViewDesc& indexBuffer) override;
    RhiResult SetRenderTargets(const RhiSetRenderTargetsDesc& desc) override;

    RhiResult SetRootConstants(const RhiSetRootConstantsDesc& desc) override;
    RhiResult SetRootDescriptorTable(const RhiSetRootDescriptorTableDesc& desc) override;
    RhiResult SetRootShaderResource(const RhiSetRootShaderResourceDesc& desc) override;
    RhiResult SetRootConstantBuffer(const RhiSetRootConstantBufferDesc& desc) override;
    RhiResult SetDescriptorHeaps(const RhiSetDescriptorHeapsDesc& desc) override;

    RhiResult ClearRenderTargetView(const RhiClearRenderTargetViewDesc& desc) override;
    RhiResult ClearDepthStencilView(const RhiClearDepthStencilViewDesc& desc) override;

    RhiResult CopyBufferRegion(const RhiCopyBufferRegionDesc& desc) override;
    RhiResult CopyTextureRegion(const RhiCopyTextureRegionDesc& desc) override;
    RhiResult TransitionResource(const RhiResourceTransitionDesc& desc) override;
    RhiResult DrawInstanced(const RhiDrawInstancedDesc& desc) override;
    RhiResult DrawIndexedInstanced(const RhiDrawIndexedInstancedDesc& desc) override;

    RhiResult Reset(const RhiCommandAllocatorHandle commandAllocator) override;
    RhiResult Close() override;

private:
    friend class D3D12Device;
    friend class D3D12CommandQueue;

    friend class GuiRenderer;

    wrl::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
};

ETH_NAMESPACE_END
