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

#include "gbufferproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/config/graphicconfig.h"
#include "graphics/resources/mesh.h"
#include "graphics/resources/material.h"

#include "graphics/shaders/common/globalconstants.h"
#include "graphics/shaders/common/material.h"

DEFINE_GFX_PA(GBufferProducer)
DEFINE_GFX_DS(GBufferDepthStencil)
DEFINE_GFX_RT(GBufferTexture0) // [Albedo.x,   Albedo.y,   Albedo.z,   ValidFlag]
DEFINE_GFX_RT(GBufferTexture1) // [Position.x, Position.y, Position.z, Normal.x]
DEFINE_GFX_RT(GBufferTexture2) // [Normal.y,   zormal.z,   Velocity.x, Velocity.y]

DECLARE_GFX_CB(GlobalConstantsRing)

void Ether::Graphics::GBufferProducer::Initialize(ResourceContext& rc)
{
    CreateShaders();
    CreateRootSignature();
    CreatePipelineState(rc);
}

void Ether::Graphics::GBufferProducer::GetInputOutput(ScheduleContext& schedule)
{
    ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();
    schedule.NewDS(ACCESS_GFX_DS(GBufferDepthStencil), resolution.x, resolution.y, DepthBufferFormat);
    schedule.NewRT(ACCESS_GFX_RT(GBufferTexture0), resolution.x, resolution.y, RhiFormat::R32G32B32A32Float);
    schedule.NewRT(ACCESS_GFX_RT(GBufferTexture1), resolution.x, resolution.y, RhiFormat::R32G32B32A32Float);
    schedule.NewRT(ACCESS_GFX_RT(GBufferTexture2), resolution.x, resolution.y, RhiFormat::R32G32B32A32Float);

    schedule.Read(ACCESS_GFX_CB(GlobalConstantsRing));
}

void Ether::Graphics::GBufferProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    ETH_MARKER_EVENT("GBufferProducer - Render");
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();

    ctx.PushMarker("Clear");
    ctx.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::RenderTarget);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_RT(GBufferTexture0)), RhiResourceState::RenderTarget);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_RT(GBufferTexture1)), RhiResourceState::RenderTarget);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_RT(GBufferTexture2)), RhiResourceState::RenderTarget);
    ctx.ClearColor(gfxDisplay.GetBackBufferRtv(), config.GetClearColor());
    ctx.ClearColor(*ACCESS_GFX_RT(GBufferTexture0));
    ctx.ClearColor(*ACCESS_GFX_RT(GBufferTexture1));
    ctx.ClearColor(*ACCESS_GFX_RT(GBufferTexture2));
    ctx.ClearDepthStencil(*ACCESS_GFX_DS(GBufferDepthStencil), 1.0);
    ctx.PopMarker();

    ctx.PushMarker("Draw GBuffer Geometry");
    ctx.SetViewport(gfxDisplay.GetViewport());
    ctx.SetScissorRect(gfxDisplay.GetScissorRect());
    ctx.SetPrimitiveTopology(RhiPrimitiveTopology::TriangleList);
    ctx.SetDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
    ctx.SetGraphicRootSignature(*m_RootSignature);
    ctx.SetPipelineState(rc.GetPipelineState(*m_PsoDesc));
    ctx.SetGraphicsRootConstantBufferView(0, rc.GetResource(ACCESS_GFX_CB(GlobalConstantsRing))->GetGpuAddress() + gfxDisplay.GetBackBufferIndex() * sizeof(Shader::GlobalConstants));

    RhiRenderTargetView rtvs[] = { *ACCESS_GFX_RT(GBufferTexture0),
                                   *ACCESS_GFX_RT(GBufferTexture1),
                                   *ACCESS_GFX_RT(GBufferTexture2),
                                   gfxDisplay.GetBackBufferRtv() };
    ctx.SetRenderTargets(rtvs, sizeof(rtvs) / sizeof(rtvs[0]), &(*ACCESS_GFX_DS(GBufferDepthStencil)));

    const VisualBatch& visualBatch = GraphicCore::GetGraphicRenderer().GetRenderData().m_VisualBatch;

    for (int i = 0; i < visualBatch.m_Visuals.size(); ++i)
    {
        ETH_MARKER_EVENT("Draw Meshes");
        Visual visual = visualBatch.m_Visuals[i];

        Shader::Material instanceMaterial;
        instanceMaterial.m_BaseColor = visual.m_Material->GetBaseColor();
        instanceMaterial.m_SpecularColor = visual.m_Material->GetSpecularColor();
        instanceMaterial.m_AlbedoTextureIndex = GraphicCore::GetBindlessDescriptorManager().GetDescriptorIndex(visual.m_Material->GetAlbedoTextureID());

        auto alloc = GetFrameAllocator().Allocate({ sizeof(Shader::Material), 256 });
        memcpy(alloc->GetCpuHandle(), &instanceMaterial, sizeof(Shader::Material));

        ctx.SetGraphicsRootConstantBufferView(1, ((UploadBufferAllocation&)(*alloc)).GetGpuAddress());
        ctx.SetVertexBuffer(visual.m_Mesh->GetVertexBufferView());
        ctx.SetIndexBuffer(visual.m_Mesh->GetIndexBufferView());
        ctx.DrawIndexedInstanced(visual.m_Mesh->GetNumIndices(), 1);
    }

    ctx.PopMarker();
}

void Ether::Graphics::GBufferProducer::CreateShaders()
{
    RhiDevice& gfxDevice = GraphicCore::GetDevice();
    m_VertexShader = gfxDevice.CreateShader({ "gbuffer.hlsl", "VS_Main", RhiShaderType::Vertex });
    m_PixelShader = gfxDevice.CreateShader({ "gbuffer.hlsl", "PS_Main", RhiShaderType::Pixel });

    // Manually compile shader since raytracing PSO caching has not been implemented yet
    m_VertexShader->Compile();
    m_PixelShader->Compile();

    GraphicCore::GetShaderDaemon().RegisterShader(*m_VertexShader);
    GraphicCore::GetShaderDaemon().RegisterShader(*m_PixelShader);
}

void Ether::Graphics::GBufferProducer::CreateRootSignature()
{
    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(2, 1);
    rsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All);
    rsDesc->SetAsConstantBufferView(1, 1, RhiShaderVisibility::All);
    rsDesc->SetAsSampler(0, GraphicCore::GetGraphicCommon().m_BilinearSampler_Wrap, RhiShaderVisibility::All);
    rsDesc->SetFlags(RhiRootSignatureFlag::AllowIAInputLayout | RhiRootSignatureFlag::DirectlyIndexed);
    m_RootSignature = rsDesc->Compile("GBufferProducer Root Signature");
}

void Ether::Graphics::GBufferProducer::CreatePipelineState(ResourceContext& rc)
{
    m_PsoDesc = GraphicCore::GetDevice().CreatePipelineStateDesc();
    m_PsoDesc->SetVertexShader(*m_VertexShader);
    m_PsoDesc->SetPixelShader(*m_PixelShader);
    RhiFormat formats[] = { RhiFormat::R32G32B32A32Float,
                            RhiFormat::R32G32B32A32Float,
                            RhiFormat::R32G32B32A32Float,
                            BackBufferFormat };
    m_PsoDesc->SetRenderTargetFormats(formats, sizeof(formats) / sizeof(formats[0]));
    m_PsoDesc->SetRootSignature(*m_RootSignature);
    m_PsoDesc->SetInputLayout(
        VertexFormats::PositionNormalTangentBitangentTexcoord::s_InputElementDesc,
        VertexFormats::PositionNormalTangentBitangentTexcoord::s_NumElements);
    m_PsoDesc->SetDepthTargetFormat(DepthBufferFormat);
    m_PsoDesc->SetDepthStencilState(GraphicCore::GetGraphicCommon().m_DepthStateReadWrite);
    rc.RegisterPipelineState("GBufferProducer Pipeline State", *m_PsoDesc);
}
