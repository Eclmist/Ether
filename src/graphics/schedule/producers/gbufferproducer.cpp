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
DEFINE_GFX_RT(GBufferTexture1) // [Position.x, Position.y, Position.z, Roughness]
DEFINE_GFX_RT(GBufferTexture2) // [Normal.x,   Normal.y,   Normal.z,   Metalness]
DEFINE_GFX_RT(GBufferTexture3) // [Velocity.x, Velocity.y, Unused,     Unused]
DEFINE_GFX_SR(GBufferTexture0)
DEFINE_GFX_SR(GBufferTexture1)
DEFINE_GFX_SR(GBufferTexture2)
DEFINE_GFX_SR(GBufferTexture3)

DECLARE_GFX_CB(GlobalRingBuffer)

Ether::Graphics::GBufferProducer::GBufferProducer()
    : GraphicProducer("GBufferProducer")
{
}

void Ether::Graphics::GBufferProducer::Initialize(ResourceContext& rc)
{
    CreateShaders();
    CreateRootSignature();
    CreatePipelineState(rc);
}

void Ether::Graphics::GBufferProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();
    schedule.NewDS(ACCESS_GFX_DS(GBufferDepthStencil), resolution.x, resolution.y, DepthBufferFormat);
    schedule.NewRT(ACCESS_GFX_RT(GBufferTexture0), resolution.x, resolution.y, RhiFormat::R32G32B32A32Float);
    schedule.NewRT(ACCESS_GFX_RT(GBufferTexture1), resolution.x, resolution.y, RhiFormat::R32G32B32A32Float);
    schedule.NewRT(ACCESS_GFX_RT(GBufferTexture2), resolution.x, resolution.y, RhiFormat::R32G32B32A32Float);
    schedule.NewRT(ACCESS_GFX_RT(GBufferTexture3), resolution.x, resolution.y, RhiFormat::R32G32B32A32Float);
    schedule.NewSR(ACCESS_GFX_SR(GBufferTexture0), resolution.x, resolution.y, RhiFormat::R32G32B32A32Float, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(GBufferTexture1), resolution.x, resolution.y, RhiFormat::R32G32B32A32Float, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(GBufferTexture2), resolution.x, resolution.y, RhiFormat::R32G32B32A32Float, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(GBufferTexture3), resolution.x, resolution.y, RhiFormat::R32G32B32A32Float, RhiResourceDimension::Texture2D);

    schedule.Read(ACCESS_GFX_CB(GlobalRingBuffer));
}

void Ether::Graphics::GBufferProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();
    const VisualBatch& visualBatch = GraphicCore::GetGraphicRenderer().GetRenderData().m_VisualBatch;

    ctx.PushMarker("Clear");
    ctx.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::RenderTarget);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_RT(GBufferTexture0)), RhiResourceState::RenderTarget);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_RT(GBufferTexture1)), RhiResourceState::RenderTarget);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_RT(GBufferTexture2)), RhiResourceState::RenderTarget);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_RT(GBufferTexture3)), RhiResourceState::RenderTarget);
    ctx.ClearColor(*ACCESS_GFX_RT(GBufferTexture0));
    ctx.ClearColor(*ACCESS_GFX_RT(GBufferTexture1));
    ctx.ClearColor(*ACCESS_GFX_RT(GBufferTexture2));
    ctx.ClearColor(*ACCESS_GFX_RT(GBufferTexture3));
    ctx.ClearDepthStencil(*ACCESS_GFX_DS(GBufferDepthStencil), 1.0);
    ctx.PopMarker();

    ctx.PushMarker("Draw Geometry");
    ctx.SetViewport(gfxDisplay.GetViewport());
    ctx.SetScissorRect(gfxDisplay.GetScissorRect());
    ctx.SetPrimitiveTopology(RhiPrimitiveTopology::TriangleList);
    ctx.SetSrvCbvUavDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
    ctx.SetSamplerDescriptorHeap(GraphicCore::GetSamplerAllocator().GetDescriptorHeap());
    ctx.SetGraphicRootSignature(*m_RootSignature);
    ctx.SetGraphicPipelineState((RhiGraphicPipelineState&)rc.GetPipelineState(*m_PsoDesc));

    uint64_t ringBufferOffset = gfxDisplay.GetBackBufferIndex() * sizeof(Shader::GlobalConstants);
    ctx.SetGraphicsRootConstantBufferView(0, rc.GetResource(ACCESS_GFX_CB(GlobalRingBuffer))->GetGpuAddress() + ringBufferOffset);

    RhiRenderTargetView rtvs[] = { *ACCESS_GFX_RT(GBufferTexture0),
                                   *ACCESS_GFX_RT(GBufferTexture1),
                                   *ACCESS_GFX_RT(GBufferTexture2),
                                   *ACCESS_GFX_RT(GBufferTexture3) };
    
    ctx.SetRenderTargets(rtvs, sizeof(rtvs) / sizeof(rtvs[0]), &(*ACCESS_GFX_DS(GBufferDepthStencil)));


    for (int i = 0; i < visualBatch.m_Visuals.size(); ++i)
    {
        ETH_MARKER_EVENT("Draw Meshes");
        Visual visual = visualBatch.m_Visuals[i];

        Shader::Material instanceMaterial;
        instanceMaterial.m_BaseColor = visual.m_Material->GetBaseColor();
        instanceMaterial.m_SpecularColor = visual.m_Material->GetSpecularColor();
        instanceMaterial.m_AlbedoTextureIndex = GraphicCore::GetBindlessDescriptorManager().GetDescriptorIndex(visual.m_Material->GetAlbedoTextureID());
        instanceMaterial.m_NormalTextureIndex = GraphicCore::GetBindlessDescriptorManager().GetDescriptorIndex(visual.m_Material->GetNormalTextureID());
        instanceMaterial.m_RoughnessTextureIndex = GraphicCore::GetBindlessDescriptorManager().GetDescriptorIndex(visual.m_Material->GetRoughnessTextureID());
        instanceMaterial.m_MetalnessTextureIndex = GraphicCore::GetBindlessDescriptorManager().GetDescriptorIndex(visual.m_Material->GetMetalnessTextureID());

        auto alloc = GetFrameAllocator().Allocate({ sizeof(Shader::Material), 256 });
        memcpy(alloc->GetCpuHandle(), &instanceMaterial, sizeof(Shader::Material));

        ctx.SetGraphicsRootConstantBufferView(1, ((UploadBufferAllocation&)(*alloc)).GetGpuAddress());
        ctx.SetVertexBuffer(visual.m_Mesh->GetVertexBufferView());
        ctx.SetIndexBuffer(visual.m_Mesh->GetIndexBufferView());
        ctx.DrawIndexedInstanced(visual.m_Mesh->GetNumIndices(), 1);
    }

    ctx.PopMarker();
}

bool Ether::Graphics::GBufferProducer::IsEnabled()
{
    if (GraphicCore::GetGraphicRenderer().GetRenderData().m_VisualBatch.m_Visuals.empty())
        return false;

    return true;
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
    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(2, 0);
    rsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All); // (b0) GlobalConstants
    rsDesc->SetAsConstantBufferView(1, 1, RhiShaderVisibility::All); // (b1) Material
    rsDesc->SetFlags(RhiRootSignatureFlag::AllowIAInputLayout | RhiRootSignatureFlag::DirectlyIndexed);
    m_RootSignature = rsDesc->Compile((GetName() + " Root Signature").c_str());
}

void Ether::Graphics::GBufferProducer::CreatePipelineState(ResourceContext& rc)
{
    RhiFormat formats[] = { RhiFormat::R32G32B32A32Float,
                            RhiFormat::R32G32B32A32Float,
                            RhiFormat::R32G32B32A32Float,
                            RhiFormat::R32G32B32A32Float };
    m_PsoDesc = GraphicCore::GetDevice().CreateGraphicPipelineStateDesc();
    m_PsoDesc->SetVertexShader(*m_VertexShader);
    m_PsoDesc->SetPixelShader(*m_PixelShader);
    m_PsoDesc->SetRenderTargetFormats(formats, sizeof(formats) / sizeof(formats[0]));
    m_PsoDesc->SetRootSignature(*m_RootSignature);
    m_PsoDesc->SetInputLayout(VertexFormats::PositionNormalTangentBitangentTexcoord::s_InputElementDesc, VertexFormats::PositionNormalTangentBitangentTexcoord::s_NumElements);
    m_PsoDesc->SetDepthTargetFormat(DepthBufferFormat);
    m_PsoDesc->SetDepthStencilState(GraphicCore::GetGraphicCommon().m_DepthStateReadWrite);
    rc.RegisterPipelineState((GetName() + " Pipeline State").c_str(), *m_PsoDesc);
}

