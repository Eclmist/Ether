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

namespace Ether::Graphics
{
class CommandAllocatorPool;

class RhiCommandList : public NonCopyable, public NonMovable
{
public:
    RhiCommandList(RhiCommandType type);
    virtual ~RhiCommandList() = default;

public:
    virtual void Reset() = 0;
    virtual void Close() = 0;

    // Markers
    virtual void SetMarker(const std::string& name) = 0;
    virtual void PushMarker(const std::string& name) = 0;
    virtual void PopMarker() = 0;

    // Common
    virtual void SetDescriptorHeaps(const RhiDescriptorHeap& descriptorHeap) = 0;
    virtual void SetPipelineState(const RhiPipelineState& pso) = 0;

    // Graphics
    virtual void SetViewport(const RhiViewportDesc& viewport) = 0;
    virtual void SetScissorRect(const RhiScissorDesc& scissor) = 0;
    virtual void SetVertexBuffer(const RhiVertexBufferViewDesc& vertexBuffer) = 0;
    virtual void SetIndexBuffer(const RhiIndexBufferViewDesc& indexBuffer) = 0;
    virtual void SetPrimitiveTopology(const RhiPrimitiveTopology& primitiveTopology) = 0;
    virtual void SetStencilRef(const RhiStencilValue& val) = 0;
    virtual void SetRenderTargets(const RhiRenderTargetView* rtvs, uint32_t numRtvs, const RhiDepthStencilView* dsv) = 0;

    // Graphics Shader Data
    virtual void SetGraphicRootSignature(const RhiRootSignature& rootSignature) = 0;
    virtual void SetGraphicsRootConstant(uint32_t rootParameterIndex, uint32_t data, uint32_t destOffset) = 0;
    virtual void SetGraphicsRootConstantBufferView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr) = 0;
    virtual void SetGraphicsRootShaderResourceView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr) = 0;
    virtual void SetGraphicsRootUnorderedAccessView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr) = 0;
    virtual void SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex, RhiGpuAddress baseAddress) = 0;

    // Compute Shader Data
    virtual void SetComputeRootSignature(const RhiRootSignature& rootSignature) = 0;
    virtual void SetComputeRootConstant(uint32_t rootParameterIndex, uint32_t data, uint32_t destOffset) = 0;
    virtual void SetComputeRootConstantBufferView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr) = 0;
    virtual void SetComputeRootShaderResourceView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr) = 0;
    virtual void SetComputeRootUnorderedAccessView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr) = 0;
    virtual void SetComputeRootDescriptorTable(uint32_t rootParameterIndex, RhiGpuAddress baseAddress) = 0;

    // Raytracing
    virtual void BuildAccelerationStructure(const RhiAccelerationStructure& as) = 0;
    virtual void SetRaytracingPipelineState(const RhiRaytracingPipelineState& pso) = 0;

    // Barriers
    virtual void InsertUavBarrier(const RhiResource& uavResource) = 0;
    virtual void TransitionResource(RhiResource& resource, RhiResourceState newState) = 0;
    virtual void CopyResource(const RhiResource& src, RhiResource& dest) = 0;
    virtual void CopyBufferRegion(const RhiResource& src, RhiResource& dest, uint32_t size, uint32_t srcOffset, uint32_t destOffset) = 0;
    virtual void CopyTexture(RhiResource& scratch, RhiResource& dest, void** data, uint32_t numMips, uint32_t width, uint32_t height, uint32_t bytesPerPixel) = 0;

    // Dispatches
    virtual void ClearRenderTargetView(const RhiRenderTargetView rtv, const ethVector4& clearColor) = 0;
    virtual void ClearDepthStencilView(const RhiDepthStencilView dsv, float depth, float stencil) = 0;
    virtual void DrawInstanced(uint32_t numVert, uint32_t numInst, uint32_t firstVert, uint32_t firstInst) = 0;
    virtual void DrawIndexedInstanced(uint32_t numIndices, uint32_t numInst, uint32_t firstIdx, uint32_t stride, uint32_t firstInst) = 0;
    virtual void DispatchRays(uint32_t x, uint32_t y, uint32_t z, const RhiResource* bindTable) = 0;

public:
    RhiCommandType GetType() const { return m_Type; }

protected:
    RhiCommandType m_Type;
    RhiCommandQueue* m_CommandQueue;
    CommandAllocatorPool* m_CommandAllocatorPool;
    RhiCommandAllocator* m_CommandAllocator;
};
} // namespace Ether::Graphics
