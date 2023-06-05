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

#include "graphics/context/graphiccontext.h"
#include "graphics/graphiccore.h"

#include "graphics/rhi/rhicommandlist.h"
#include "graphics/rhi/rhidevice.h"
#include "graphics/rhi/rhiresourceviews.h"

Ether::Graphics::GraphicContext::GraphicContext(const std::string& contextName)
    : CommandContext(RhiCommandType::Graphic, contextName)
    , m_ViewMatrix()
    , m_ProjectionMatrix()
    , m_EyeDirection()
    , m_EyePosition()
    , m_Viewport()
    , m_VisualBatch()
{
}

void Ether::Graphics::GraphicContext::SetViewMatrix(ethMatrix4x4 viewMatrix)
{
    m_ViewMatrix = viewMatrix;
}

void Ether::Graphics::GraphicContext::SetProjectionMatrix(ethMatrix4x4 projectionMatrix)
{
    m_ProjectionMatrix = projectionMatrix;
}

void Ether::Graphics::GraphicContext::SetEyeDirection(ethVector3 eyeDirection)
{
    m_EyeDirection = eyeDirection;
}

void Ether::Graphics::GraphicContext::SetEyePosition(ethVector3 eyePosition)
{
    m_EyePosition = eyePosition;
}

void Ether::Graphics::GraphicContext::SetVisualBatch(const VisualBatch& visualBatch)
{
    m_VisualBatch = visualBatch;
}

void Ether::Graphics::GraphicContext::SetViewport(const RhiViewportDesc& viewport)
{
    m_CommandList->SetViewport(viewport);
}

void Ether::Graphics::GraphicContext::SetScissorRect(const RhiScissorDesc& scissor)
{
    m_CommandList->SetScissorRect(scissor);
}

void Ether::Graphics::GraphicContext::SetVertexBuffer(const RhiVertexBufferViewDesc& vertexBuffer)
{
    m_CommandList->SetVertexBuffer(vertexBuffer);
}

void Ether::Graphics::GraphicContext::SetIndexBuffer(const RhiIndexBufferViewDesc& indexBuffer)
{
    m_CommandList->SetIndexBuffer(indexBuffer);
}

void Ether::Graphics::GraphicContext::SetPrimitiveTopology(const RhiPrimitiveTopology& topology)
{
    m_CommandList->SetPrimitiveTopology(topology);
}

void Ether::Graphics::GraphicContext::SetRenderTarget(const RhiRenderTargetView& rtv, const RhiDepthStencilView* dsv)
{
    RhiSetRenderTargetsDesc desc = {};
    desc.m_NumRtv = 1;
    desc.m_RtvHandles[0] = &rtv;
    desc.m_DsvHandle = dsv;

    m_CommandList->SetRenderTargets(desc);
}

void Ether::Graphics::GraphicContext::SetGraphicRootSignature(const RhiRootSignature& rootSignature)
{
    m_CommandList->SetGraphicRootSignature(rootSignature);
}

void Ether::Graphics::GraphicContext::SetGraphicsRootConstant(
    uint32_t rootParameterIndex,
    uint32_t data,
    uint32_t destOffset)
{
    m_CommandList->SetGraphicsRootConstant(rootParameterIndex, data, destOffset);
}

void Ether::Graphics::GraphicContext::SetGraphicsRootConstantBufferView(
    uint32_t rootParameterIndex,
    RhiGpuAddress resourceAddr)
{
    m_CommandList->SetGraphicsRootConstantBufferView(rootParameterIndex, resourceAddr);
}

void Ether::Graphics::GraphicContext::SetGraphicsRootShaderResourceView(
    uint32_t rootParameterIndex,
    RhiGpuAddress resourceAddr)
{
    m_CommandList->SetGraphicsRootShaderResourceView(rootParameterIndex, resourceAddr);
}

void Ether::Graphics::GraphicContext::SetGraphicsRootUnorderedAccessView(
    uint32_t rootParameterIndex,
    RhiGpuAddress resourceAddr)

{
    m_CommandList->SetGraphicsRootUnorderedAccessView(rootParameterIndex, resourceAddr);
}

void Ether::Graphics::GraphicContext::SetGraphicsRootDescriptorTable(
    uint32_t rootParameterIndex,
    RhiGpuAddress baseAddress)
{
    m_CommandList->SetGraphicsRootDescriptorTable(rootParameterIndex, baseAddress);
}

void Ether::Graphics::GraphicContext::ClearColor(RhiRenderTargetView& rtv, const ethVector4& color)
{
    RhiClearRenderTargetViewDesc desc = {};
    desc.m_ClearColor = color;
    desc.m_RtvHandle = &rtv;
    m_CommandList->ClearRenderTargetView(desc);
}

void Ether::Graphics::GraphicContext::ClearDepthStencil(RhiDepthStencilView& dsv, float depth, float stencil)
{
    RhiClearDepthStencilViewDesc desc = {};
    desc.m_ClearDepth = depth;
    desc.m_ClearStencil = stencil;
    desc.m_DsvHandle = &dsv;
    m_CommandList->ClearDepthStencilView(desc);
}

void Ether::Graphics::GraphicContext::DrawIndexedInstanced(uint32_t numIndices, uint32_t numInstances)
{
    RhiDrawIndexedInstancedDesc desc = {};
    desc.m_IndexCount = numIndices;
    desc.m_InstanceCount = numInstances;
    desc.m_FirstIndex = 0;
    desc.m_FirstInstance = 0;
    desc.m_VertexOffset = 0;

    m_CommandList->DrawIndexedInstanced(desc);
}

void Ether::Graphics::GraphicContext::DrawInstanced(uint32_t numVertices, uint32_t numInstances)
{
    RhiDrawInstancedDesc desc = {};
    desc.m_VertexCount = numVertices;
    desc.m_InstanceCount = numInstances;
    desc.m_FirstVertex = 0;
    desc.m_FirstInstance = 0;

    m_CommandList->DrawInstanced(desc);
}

void Ether::Graphics::GraphicContext::DispatchRays(const RhiDispatchRaysDesc& desc)
{
    m_CommandList->DispatchRays(desc);
}

