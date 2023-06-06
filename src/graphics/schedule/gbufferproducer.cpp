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

#include "graphics/schedule/gbufferproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/config/graphicconfig.h"

#include "graphics/resources/mesh.h"
#include "graphics/resources/material.h"

#include "graphics/shaders/common/globalconstants.h"

namespace Ether::Graphics::RhiLinkSpace
{
extern RhiGpuAddress g_GlobalConstantsCbv;
}

void Ether::Graphics::GBufferProducer::Initialize(ResourceContext& resourceContext)
{
    CreateUploadBufferAllocators();
    CreateShaders();
    CreateRootSignature();
    CreatePipelineState(resourceContext);
}

void Ether::Graphics::GBufferProducer::FrameSetup(ResourceContext& resourceContext)
{
    m_DepthBuffer = resourceContext.CreateDepthStencilResource("GBuffer - Depth Texture", GraphicCore::GetGraphicConfig().GetResolution(), DepthBufferFormat);
    m_AlbedoTexture = resourceContext.CreateTexture2DResource("GBuffer - Albedo Texture", GraphicCore::GetGraphicConfig().GetResolution(), RhiFormat::R8G8B8A8Unorm);
    m_PositionTexture = resourceContext.CreateTexture2DResource("GBuffer - Position Texture", GraphicCore::GetGraphicConfig().GetResolution(), RhiFormat::R32G32B32A32Float);
    m_NormalTexture = resourceContext.CreateTexture2DResource("GBuffer - Normal Texture", GraphicCore::GetGraphicConfig().GetResolution(), RhiFormat::R32G32B32A32Float);

    m_DepthDsv = resourceContext.CreateDepthStencilView("GBuffer - DSV", m_DepthBuffer, DepthBufferFormat);
    m_AlbedoRtv = resourceContext.CreateRenderTargetView("GBuffer - Albedo RTV", m_AlbedoTexture, RhiFormat::R8G8B8A8Unorm);
    m_PositionRtv = resourceContext.CreateRenderTargetView("GBuffer - Position RTV", m_PositionTexture, RhiFormat::R32G32B32A32Float);
    m_NormalRtv = resourceContext.CreateRenderTargetView("GBuffer - Normal RTV", m_NormalTexture, RhiFormat::R32G32B32A32Float);
}

void Ether::Graphics::GBufferProducer::Render(GraphicContext& graphicContext, ResourceContext& resourceContext)
{
    ETH_MARKER_EVENT("Temp Frame Dump - Render");
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();

    graphicContext.PushMarker("Clear");
    graphicContext.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::RenderTarget);
    graphicContext.TransitionResource(*m_AlbedoTexture, RhiResourceState::RenderTarget);
    graphicContext.TransitionResource(*m_PositionTexture, RhiResourceState::RenderTarget);
    graphicContext.TransitionResource(*m_NormalTexture, RhiResourceState::RenderTarget);
    graphicContext.ClearColor(gfxDisplay.GetBackBufferRtv(), config.GetClearColor());
    graphicContext.ClearDepthStencil(m_DepthDsv, 1.0);
    graphicContext.PopMarker();

    graphicContext.PushMarker("Draw GBuffer Geometry");
    graphicContext.SetViewport(gfxDisplay.GetViewport());
    graphicContext.SetScissorRect(gfxDisplay.GetScissorRect());
    graphicContext.SetPrimitiveTopology(RhiPrimitiveTopology::TriangleList);
    graphicContext.SetDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
    graphicContext.SetGraphicRootSignature(*m_RootSignature);
    graphicContext.SetComputeRootSignature(*m_RootSignature);
    graphicContext.SetPipelineState(resourceContext.GetPipelineState(*m_PsoDesc));
    graphicContext.SetGraphicsRootConstantBufferView(0, RhiLinkSpace::g_GlobalConstantsCbv);
    graphicContext.SetComputeRootConstantBufferView(0, RhiLinkSpace::g_GlobalConstantsCbv);

    RhiRenderTargetView rtvs[] = { m_AlbedoRtv, m_PositionRtv, m_NormalRtv, gfxDisplay.GetBackBufferRtv() };
    graphicContext.SetRenderTargets(rtvs, sizeof(rtvs) / sizeof(rtvs[0]), &m_DepthDsv);

    const VisualBatch& visualBatch = graphicContext.GetVisualBatch();

    for (int i = 0; i < visualBatch.m_Visuals.size(); ++i)
    {
        ETH_MARKER_EVENT("Draw Meshes");
        Visual visual = visualBatch.m_Visuals[i];

        auto alloc = m_FrameLocalUploadBuffer[gfxDisplay.GetBackBufferIndex()]->Allocate({ sizeof(Material), 256 });
        memcpy(alloc->GetCpuHandle(), visual.m_Material, sizeof(Material));

        graphicContext.SetGraphicsRootConstantBufferView(1, ((UploadBufferAllocation&)(*alloc)).GetGpuAddress());
        graphicContext.SetVertexBuffer(visual.m_Mesh->GetVertexBufferView());
        graphicContext.SetIndexBuffer(visual.m_Mesh->GetIndexBufferView());
        graphicContext.DrawIndexedInstanced(visual.m_Mesh->GetNumIndices(), 1);
    }

    graphicContext.PopMarker();
}

void Ether::Graphics::GBufferProducer::Reset()
{
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    m_FrameLocalUploadBuffer[gfxDisplay.GetBackBufferIndex()]->Reset();
}

void Ether::Graphics::GBufferProducer::CreateUploadBufferAllocators()
{
    for (int i = 0; i < MaxSwapChainBuffers; ++i)
        m_FrameLocalUploadBuffer[i] = std::make_unique<UploadBufferAllocator>(_2MiB);
}

void Ether::Graphics::GBufferProducer::CreateShaders()
{
    RhiDevice& gfxDevice = GraphicCore::GetDevice();
    m_VertexShader = gfxDevice.CreateShader({ "gbuffer.hlsl", "VS_Main", RhiShaderType::Vertex });
    m_PixelShader = gfxDevice.CreateShader({ "gbuffer.hlsl", "PS_Main", RhiShaderType::Pixel });

    if (GraphicCore::GetGraphicConfig().GetUseShaderDaemon())
    {
        GraphicCore::GetShaderDaemon().RegisterShader(*m_VertexShader);
        GraphicCore::GetShaderDaemon().RegisterShader(*m_PixelShader);
    }
}

void Ether::Graphics::GBufferProducer::CreateRootSignature()
{
    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(2, 0);
    rsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All);
    rsDesc->SetAsConstantBufferView(1, 1, RhiShaderVisibility::All);
    rsDesc->SetFlags(RhiRootSignatureFlag::AllowIAInputLayout);
    m_RootSignature = rsDesc->Compile("GBufferProducer Root Signature");
}

void Ether::Graphics::GBufferProducer::CreatePipelineState(ResourceContext& resourceContext)
{
    m_PsoDesc = GraphicCore::GetDevice().CreatePipelineStateDesc();
    m_PsoDesc->SetVertexShader(*m_VertexShader);
    m_PsoDesc->SetPixelShader(*m_PixelShader);

    RhiFormat formats[] = { RhiFormat::R8G8B8A8Unorm, RhiFormat::R32G32B32A32Float, RhiFormat::R32G32B32A32Float, RhiFormat::R8G8B8A8Unorm };
    m_PsoDesc->SetRenderTargetFormats(formats, sizeof(formats) / sizeof(formats[0]));
    m_PsoDesc->SetRootSignature(*m_RootSignature);
    m_PsoDesc->SetInputLayout(
        VertexFormats::PositionNormalTangentBitangentTexcoord::s_InputElementDesc,
        VertexFormats::PositionNormalTangentBitangentTexcoord::s_NumElements);
    m_PsoDesc->SetDepthTargetFormat(DepthBufferFormat);
    m_PsoDesc->SetDepthStencilState(GraphicCore::GetGraphicCommon().m_DepthStateReadWrite);
    resourceContext.RegisterPipelineState("GBufferProducer Pipeline State", *m_PsoDesc);
}
