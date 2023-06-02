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
    Dx12CommandList(RhiCommandType type)
        : RhiCommandList(type)
    {
    }
    ~Dx12CommandList() override = default;

public:
    void SetMarker(const std::string& name) override;
    void PushMarker(const std::string& name) override;
    void PopMarker() override;

    void SetViewport(RhiViewportDesc viewport) override;
    void SetScissorRect(RhiScissorDesc scissor) override;
    void SetStencilRef(RhiStencilValue val) override;
    void SetPrimitiveTopology(RhiPrimitiveTopology primitiveTopology) override;
    void SetVertexBuffer(RhiVertexBufferViewDesc vertexBuffer) override;
    void SetIndexBuffer(RhiIndexBufferViewDesc indexBuffer) override;
    void SetRenderTargets(RhiSetRenderTargetsDesc desc) override;
    void SetDescriptorHeaps(RhiSetDescriptorHeapsDesc desc) override;
    void SetGraphicRootSignature(const RhiRootSignature& rootSignature) override;
    void SetPipelineState(const RhiPipelineState& pso) override;

    // RhiResult SetRootConstant(RhiSetRootConstantDesc desc) override;
    // RhiResult SetRootConstants(RhiSetRootConstantsDesc desc) override;
    // RhiResult SetRootDescriptorTable(RhiSetRootDescriptorTableDesc desc) override;
    // RhiResult SetRootShaderResource(RhiSetRootShaderResourceDesc desc) override;
    void SetGraphicsRootConstantBuffer(uint32_t bindSlot, RhiGpuAddress resourceAddr) override;

    void ClearRenderTargetView(RhiClearRenderTargetViewDesc desc) override;
    void ClearDepthStencilView(RhiClearDepthStencilViewDesc desc) override;

    void TransitionResource(RhiResourceTransitionDesc desc) override;
    void CopyBufferRegion(RhiCopyBufferRegionDesc desc) override;
    void CopyTextureRegion(RhiCopyTextureRegionDesc desc) override;
    void DrawInstanced(RhiDrawInstancedDesc desc) override;
    void DrawIndexedInstanced(RhiDrawIndexedInstancedDesc desc) override;

    void Reset(const RhiCommandAllocator& commandAllocator) override;
    void Close() override;

private:
    friend class Dx12Device;
    friend class Dx12CommandQueue;
    friend class Dx12ImguiWrapper;

    wrl::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
};
} // namespace Ether::Graphics
