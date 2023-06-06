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
#include "graphics/rhi/rhicommandlist.h"
#include "graphics/rhi/dx12/dx12includes.h"

namespace Ether::Graphics
{
class Dx12CommandList : public RhiCommandList
{
public:
    Dx12CommandList(RhiCommandType type);
    ~Dx12CommandList() override = default;

public:
    void Reset() override;
    void Close() override;

    // Markers
    void SetMarker(const std::string& name) override;
    void PushMarker(const std::string& name) override;
    void PopMarker() override;

    // Common
    void SetDescriptorHeaps(const RhiSetDescriptorHeapsDesc& desc) override;
    void SetPipelineState(const RhiPipelineState& pso) override;

    // Graphics
    void SetViewport(const RhiViewportDesc& viewport) override;
    void SetScissorRect(const RhiScissorDesc& scissor) override;
    void SetVertexBuffer(const RhiVertexBufferViewDesc& vertexBuffer) override;
    void SetIndexBuffer(const RhiIndexBufferViewDesc& indexBuffer) override;
    void SetPrimitiveTopology(const RhiPrimitiveTopology& primitiveTopology) override;
    void SetStencilRef(const RhiStencilValue& val) override;
    void SetRenderTargets(const RhiRenderTargetView* rtvs, uint32_t numRtvs, const RhiDepthStencilView* dsv) override;

    // Graphics Shader Data
    void SetGraphicRootSignature(const RhiRootSignature& rootSignature) override;
    void SetGraphicsRootConstant(uint32_t rootParameterIndex, uint32_t data, uint32_t destOffset) override;
    void SetGraphicsRootConstantBufferView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr) override;
    void SetGraphicsRootShaderResourceView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr) override;
    void SetGraphicsRootUnorderedAccessView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr) override;
    void SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex, RhiGpuAddress baseAddress) override;

    // Compute Shader Data
    void SetComputeRootSignature(const RhiRootSignature& rootSignature) override;
    void SetComputeRootConstant(uint32_t rootParameterIndex, uint32_t data, uint32_t destOffset) override;
    void SetComputeRootConstantBufferView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr) override;
    void SetComputeRootShaderResourceView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr) override;
    void SetComputeRootUnorderedAccessView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr) override;
    void SetComputeRootDescriptorTable(uint32_t rootParameterIndex, RhiGpuAddress baseAddress) override;

    // Raytracing
    void BuildAccelerationStructure(const RhiAccelerationStructure& as) override;
    void SetRaytracingPipelineState(const RhiRaytracingPipelineState& pso) override;

    // Barriers
    void InsertUavBarrier(const RhiResource& uavResource) override;
    void TransitionResource(RhiResource& resource, RhiResourceState newState) override;
    void CopyResource(const RhiResource& src, RhiResource& dest) override;
    void CopyBufferRegion(const RhiCopyBufferRegionDesc& desc) override;
    void CopyTextureRegion(const RhiCopyTextureRegionDesc& desc) override;

    // Dispatches
    void ClearRenderTargetView(const RhiClearRenderTargetViewDesc& desc) override;
    void ClearDepthStencilView(const RhiClearDepthStencilViewDesc& desc) override;
    void DrawInstanced(const RhiDrawInstancedDesc& desc) override;
    void DrawIndexedInstanced(const RhiDrawIndexedInstancedDesc& desc) override;
    void DispatchRays(uint32_t x, uint32_t y, uint32_t z, const RhiRaytracingShaderBindingTable& bindTable) override;

private:
    friend class Dx12Device;
    friend class Dx12CommandQueue;
    friend class Dx12ImguiWrapper;

    wrl::ComPtr<ID3D12GraphicsCommandList4> m_CommandList;
};
} // namespace Ether::Graphics
