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

void Ether::Graphics::GBufferProducer::Initialize(ResourceContext& rc)
{
    CreateUploadBufferAllocators();
    CreateShaders();
    CreateRootSignature();
    CreatePipelineState(rc);
}

void Ether::Graphics::GBufferProducer::FrameSetup(ResourceContext& rc)
{
    ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();

    m_DepthBuffer = &rc.CreateDepthStencilResource("GBuffer - Depth Texture", resolution, DepthBufferFormat);
    m_AlbedoTexture = &rc.CreateTexture2DResource("GBuffer - Albedo Texture", resolution, RhiFormat::R8G8B8A8Unorm);
    m_PositionTexture = &rc.CreateTexture2DResource("GBuffer - Position Texture", resolution, RhiFormat::R32G32B32A32Float);
    m_NormalTexture = &rc.CreateTexture2DResource("GBuffer - Normal Texture", resolution, RhiFormat::R32G32B32A32Float);

    m_DepthDsv = rc.CreateDepthStencilView("GBuffer - DSV", m_DepthBuffer, DepthBufferFormat);
    m_AlbedoRtv = rc.CreateRenderTargetView("GBuffer - Albedo RTV", m_AlbedoTexture, RhiFormat::R8G8B8A8Unorm);
    m_PositionRtv = rc.CreateRenderTargetView("GBuffer - Position RTV", m_PositionTexture, RhiFormat::R32G32B32A32Float);
    m_NormalRtv = rc.CreateRenderTargetView("GBuffer - Normal RTV", m_NormalTexture, RhiFormat::R32G32B32A32Float);
}

void Ether::Graphics::GBufferProducer::Render(GraphicContext& ctx, ResourceContext& rc)
{
    ETH_MARKER_EVENT("Temp Frame Dump - Render");
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();

    ctx.PushMarker("Clear");
    ctx.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::RenderTarget);
    ctx.TransitionResource(*m_AlbedoTexture, RhiResourceState::RenderTarget);
    ctx.TransitionResource(*m_PositionTexture, RhiResourceState::RenderTarget);
    ctx.TransitionResource(*m_NormalTexture, RhiResourceState::RenderTarget);
    ctx.ClearColor(gfxDisplay.GetBackBufferRtv(), config.GetClearColor());
    ctx.ClearDepthStencil(m_DepthDsv, 1.0);
    ctx.PopMarker();

    ctx.PushMarker("Draw GBuffer Geometry");
    ctx.SetViewport(gfxDisplay.GetViewport());
    ctx.SetScissorRect(gfxDisplay.GetScissorRect());
    ctx.SetPrimitiveTopology(RhiPrimitiveTopology::TriangleList);
    ctx.SetDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
    ctx.SetGraphicRootSignature(*m_RootSignature);
    ctx.SetComputeRootSignature(*m_RootSignature);
    ctx.SetPipelineState(rc.GetPipelineState(*m_PsoDesc));
    ctx.SetGraphicsRootConstantBufferView(0, RhiLinkSpace::g_GlobalConstantsCbv);
    ctx.SetComputeRootConstantBufferView(0, RhiLinkSpace::g_GlobalConstantsCbv);

    RhiRenderTargetView rtvs[] = { m_AlbedoRtv, m_PositionRtv, m_NormalRtv, gfxDisplay.GetBackBufferRtv() };
    ctx.SetRenderTargets(rtvs, sizeof(rtvs) / sizeof(rtvs[0]), &m_DepthDsv);

    const VisualBatch& visualBatch = ctx.GetVisualBatch();

    for (int i = 0; i < visualBatch.m_Visuals.size(); ++i)
    {
        ETH_MARKER_EVENT("Draw Meshes");
        Visual visual = visualBatch.m_Visuals[i];

        auto alloc = m_FrameLocalUploadBuffer[gfxDisplay.GetBackBufferIndex()]->Allocate({ sizeof(Material), 256 });
        memcpy(alloc->GetCpuHandle(), visual.m_Material, sizeof(Material));

        ctx.SetGraphicsRootConstantBufferView(1, ((UploadBufferAllocation&)(*alloc)).GetGpuAddress());
        ctx.SetVertexBuffer(visual.m_Mesh->GetVertexBufferView());
        ctx.SetIndexBuffer(visual.m_Mesh->GetIndexBufferView());
        ctx.DrawIndexedInstanced(visual.m_Mesh->GetNumIndices(), 1);
    }

    ctx.PopMarker();
}

void Ether::Graphics::GBufferProducer::Reset()
{
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
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

void Ether::Graphics::GBufferProducer::CreatePipelineState(ResourceContext& rc)
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
    rc.RegisterPipelineState("GBufferProducer Pipeline State", *m_PsoDesc);
}
