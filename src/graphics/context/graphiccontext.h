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
#include "graphics/common/renderdata.h"
#include "graphics/context/commandcontext.h"
#include "graphics/rhi/rhitypes.h"

namespace Ether::Graphics
{
class GraphicContext : public CommandContext
{
public:
    GraphicContext(const char* contextName, size_t uploadBufferSize = _4MiB);
    ~GraphicContext() = default;

public:
    // Common
    void SetRenderData(const RenderData& renderData);
    void SetViewport(const RhiViewportDesc& viewport);
    void SetScissorRect(const RhiScissorDesc& scissor);
    void SetVertexBuffer(const RhiVertexBufferViewDesc& vertexBuffer);
    void SetIndexBuffer(const RhiIndexBufferViewDesc& indexBuffer);
    void SetPrimitiveTopology(const RhiPrimitiveTopology& topology);
    void SetRenderTarget(const RhiRenderTargetView rtv, const RhiDepthStencilView* dsv = nullptr);
    void SetRenderTargets(const RhiRenderTargetView* rtvs, uint32_t numRtvs, const RhiDepthStencilView* dsv = nullptr);

    // Shader Data
    void SetGraphicRootSignature(const RhiRootSignature& rootSignature);
    void SetGraphicsRootConstant(uint32_t rootParameterIndex, uint32_t data, uint32_t destOffset);
    void SetGraphicsRootConstantBufferView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr);
    void SetGraphicsRootShaderResourceView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr);
    void SetGraphicsRootUnorderedAccessView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr);
    void SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex, RhiGpuAddress baseAddress);

    // Dispatches
    void ClearColor(RhiRenderTargetView rtv, const ethVector4& color = { 0, 0, 0, 0 });
    void ClearDepthStencil(RhiDepthStencilView dsv, float depth, float stencil = 0.0f);
    void DrawInstanced(uint32_t numVertices, uint32_t numInstances);
    void DrawIndexedInstanced(uint32_t numIndices, uint32_t numInstances);

private:
    RhiViewportDesc m_Viewport;
    RenderData m_RenderData;
};
} // namespace Ether::Graphics
