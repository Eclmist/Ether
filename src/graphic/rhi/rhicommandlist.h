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

class RHICommandList
{
public:
    RHICommandList(RHICommandListType type) : m_Type(type) {}
	virtual ~RHICommandList() = default;

public:
    virtual RHIResult SetViewport(const RHIViewportDesc& viewport) = 0;
    virtual RHIResult SetScissor(const RHIScissorDesc& scissor) = 0;
    virtual RHIResult SetPrimitiveTopology(RHIPrimitiveTopology primitiveTopology) = 0;
    virtual RHIResult SetPipelineState(const RHIPipelineStateHandle pipelineState) = 0;
    virtual RHIResult SetGraphicRootSignature(const RHIRootSignatureHandle rootSignature) = 0;
    virtual RHIResult SetVertexBuffer(const RHIVertexBufferViewDesc& vertexBuffer) = 0;
    virtual RHIResult SetIndexBuffer(const RHIIndexBufferViewDesc& indexBuffer) = 0;
    virtual RHIResult SetRenderTargets(const RHISetRenderTargetsDesc& desc) = 0;
    virtual RHIResult SetRootConstants(const RHISetRootConstantsDesc& desc) = 0;
    virtual RHIResult SetRootDescriptorTable(const RHISetRootDescriptorTableDesc& desc) = 0;
    virtual RHIResult SetDescriptorHeaps(const RHISetDescriptorHeapsDesc& desc) = 0;

    virtual RHIResult ClearRenderTargetView(const RHIClearRenderTargetViewDesc& desc) = 0;
    virtual RHIResult ClearDepthStencilView(const RHIClearDepthStencilViewDesc& desc) = 0;

    virtual RHIResult CopyBufferRegion(const RHICopyBufferRegionDesc& desc) = 0;
    virtual RHIResult TransitionResource(const RHIResourceTransitionDesc& desc) = 0;
    virtual RHIResult DrawInstanced(const RHIDrawInstancedDesc& desc) = 0;
    virtual RHIResult DrawIndexedInstanced(const RHIDrawIndexedInstancedDesc& desc) = 0;

    virtual RHIResult Reset(const RHICommandAllocatorHandle commandAllocator) = 0;
    virtual RHIResult Close() = 0;

public:
    RHICommandListType GetType() const { return m_Type; }

protected:
    RHICommandListType m_Type;
};

ETH_NAMESPACE_END

