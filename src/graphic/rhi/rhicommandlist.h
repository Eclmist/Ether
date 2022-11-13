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

ETH_NAMESPACE_BEGIN

class RhiCommandList
{
public:
    RhiCommandList(RhiCommandListType type) : m_Type(type) {}
    virtual ~RhiCommandList() = default;

public:
    virtual RhiResult SetMarker(const char* name) = 0;
    virtual RhiResult PushMarker(const char* name) = 0;
    virtual RhiResult PopMarker() = 0;

    virtual RhiResult SetViewport(const RhiViewportDesc& viewport) = 0;
    virtual RhiResult SetScissor(const RhiScissorDesc& scissor) = 0;
    virtual RhiResult SetStencilRef(const RhiStencilValue& val) = 0;
    virtual RhiResult SetPrimitiveTopology(RhiPrimitiveTopology primitiveTopology) = 0;
    virtual RhiResult SetPipelineState(const RhiPipelineStateHandle pipelineState) = 0;
    virtual RhiResult SetGraphicRootSignature(const RhiRootSignatureHandle rootSignature) = 0;
    virtual RhiResult SetVertexBuffer(const RhiVertexBufferViewDesc& vertexBuffer) = 0;
    virtual RhiResult SetIndexBuffer(const RhiIndexBufferViewDesc& indexBuffer) = 0;
    virtual RhiResult SetRenderTargets(const RhiSetRenderTargetsDesc& desc) = 0;

    virtual RhiResult SetRootConstant(const RhiSetRootConstantDesc& desc) = 0;
    virtual RhiResult SetRootConstants(const RhiSetRootConstantsDesc& desc) = 0;
    virtual RhiResult SetRootDescriptorTable(const RhiSetRootDescriptorTableDesc& desc) = 0;
    virtual RhiResult SetRootShaderResource(const RhiSetRootShaderResourceDesc& desc) = 0;
    virtual RhiResult SetRootConstantBuffer(const RhiSetRootConstantBufferDesc& desc) = 0;
    virtual RhiResult SetDescriptorHeaps(const RhiSetDescriptorHeapsDesc& desc) = 0;

    virtual RhiResult ClearRenderTargetView(const RhiClearRenderTargetViewDesc& desc) = 0;
    virtual RhiResult ClearDepthStencilView(const RhiClearDepthStencilViewDesc& desc) = 0;

    virtual RhiResult CopyBufferRegion(const RhiCopyBufferRegionDesc& desc) = 0;
    virtual RhiResult CopyTextureRegion(const RhiCopyTextureRegionDesc& desc) = 0;
    virtual RhiResult TransitionResource(const RhiResourceTransitionDesc& desc) = 0;
    virtual RhiResult DrawInstanced(const RhiDrawInstancedDesc& desc) = 0;
    virtual RhiResult DrawIndexedInstanced(const RhiDrawIndexedInstancedDesc& desc) = 0;

    virtual RhiResult Reset(const RhiCommandAllocatorHandle commandAllocator) = 0;
    virtual RhiResult Close() = 0;

public:
    RhiCommandListType GetType() const { return m_Type; }

protected:
    RhiCommandListType m_Type;
};

ETH_NAMESPACE_END

