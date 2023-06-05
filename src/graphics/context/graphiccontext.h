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
#include "graphics/common/visualbatch.h"
#include "graphics/context/commandcontext.h"
#include "graphics/rhi/rhitypes.h"

namespace Ether::Graphics
{
class ETH_GRAPHIC_DLL GraphicContext : public CommandContext
{
public:
    GraphicContext(const std::string& contextName = "Unnamed Graphic Context");
    ~GraphicContext() = default;

public:
    inline const ethMatrix4x4& GetViewMatrix() const { return m_ViewMatrix; }
    inline const ethMatrix4x4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
    inline const ethVector3& GetEyePosition() const { return m_EyePosition; }
    inline const ethVector3& GetEyeDirection() const { return m_EyeDirection; }

    inline const RhiViewportDesc& GetViewport() const { return m_Viewport; }
    inline const VisualBatch& GetVisualBatch() const { return m_VisualBatch; }

public:
    // Common
    void SetViewMatrix(ethMatrix4x4 viewMatrix);
    void SetProjectionMatrix(ethMatrix4x4 projectionMatrix);
    void SetEyeDirection(ethVector3 eyeDirection);
    void SetEyePosition(ethVector3 eyePosition);
    void SetVisualBatch(const VisualBatch& visualBatch);
    void SetViewport(const RhiViewportDesc& viewport);
    void SetScissorRect(const RhiScissorDesc& scissor);
    void SetVertexBuffer(const RhiVertexBufferViewDesc& vertexBuffer);
    void SetIndexBuffer(const RhiIndexBufferViewDesc& indexBuffer);
    void SetPrimitiveTopology(const RhiPrimitiveTopology& topology);
    void SetRenderTarget(const RhiRenderTargetView& rtv, const RhiDepthStencilView* dsv = nullptr);

    // Shader Data
    void SetGraphicRootSignature(const RhiRootSignature& rootSignature);
    void SetGraphicsRootConstant(uint32_t rootParameterIndex, uint32_t data, uint32_t destOffset);
    void SetGraphicsRootConstantBufferView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr);
    void SetGraphicsRootShaderResourceView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr);
    void SetGraphicsRootUnorderedAccessView(uint32_t rootParameterIndex, RhiGpuAddress resourceAddr);
    void SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex, RhiGpuAddress baseAddress);

    // Dispatches
    void ClearColor(RhiRenderTargetView& rtv, const ethVector4& color = { 0, 0, 0, 0 });
    void ClearDepthStencil(RhiDepthStencilView& dsv, float depth, float stencil = 0.0f);
    void DrawIndexedInstanced(uint32_t numIndices, uint32_t numInstances);
    void DrawInstanced(uint32_t numVertices, uint32_t numInstances);
    void DispatchRays(const RhiDispatchRaysDesc& desc);

private:
    ethMatrix4x4 m_ViewMatrix;
    ethMatrix4x4 m_ProjectionMatrix;
    ethVector3 m_EyeDirection;
    ethVector3 m_EyePosition;

    RhiViewportDesc m_Viewport;
    VisualBatch m_VisualBatch;
};
} // namespace Ether::Graphics
