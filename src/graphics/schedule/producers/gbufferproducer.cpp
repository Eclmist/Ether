/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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
#include "graphics/resources/staticmesh.h"
#include "graphics/resources/skinnedmesh.h"
#include "graphics/resources/material.h"

#include "graphics/shaders/common/globalconstants.h"
#include "graphics/shaders/common/instanceparams.h"

DEFINE_GFX_PA(GBufferProducer)
DEFINE_GFX_DS(SceneDepth)
DEFINE_GFX_RT(GBufferTextureA) // [BaseColor.x, BaseColor.y, BaseColor.z, MaterialID]
DEFINE_GFX_RT(GBufferTextureB) // [Normal.x,    Normal.y,    Velocity.x,  Velocity.y]
DEFINE_GFX_RT(GBufferTextureC) // [Emissive.x,  Emissive.y,  Emissive.z,  Roughness & Metalness Packed]

DEFINE_GFX_SR(SceneDepth)
DEFINE_GFX_SR(GBufferTextureA)
DEFINE_GFX_SR(GBufferTextureB)
DEFINE_GFX_SR(GBufferTextureC)

#if ETH_TOOLMODE
DEFINE_GFX_RT(MetadataBuffer)
DEFINE_GFX_SR(MetadataBuffer)
#endif

DECLARE_GFX_CB(GlobalConstants)
DECLARE_GFX_SR(MaterialTable)

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

    schedule.NewDS(ACCESS_GFX_DS(SceneDepth), resolution.x, resolution.y, DepthBufferDsvFormat);
    schedule.NewRT(ACCESS_GFX_RT(GBufferTextureA), resolution.x, resolution.y, RhiFormat::R8G8B8A8Unorm);
    schedule.NewRT(ACCESS_GFX_RT(GBufferTextureB), resolution.x, resolution.y, RhiFormat::R16G16B16A16Float);
    schedule.NewRT(ACCESS_GFX_RT(GBufferTextureC), resolution.x, resolution.y, RhiFormat::R16G16B16A16Float);
    schedule.NewSR(ACCESS_GFX_SR(SceneDepth), resolution.x, resolution.y, DepthBufferSrvFormat, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(GBufferTextureA), resolution.x, resolution.y, RhiFormat::R8G8B8A8Unorm, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(GBufferTextureB), resolution.x, resolution.y, RhiFormat::R16G16B16A16Float, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(GBufferTextureC), resolution.x, resolution.y, RhiFormat::R16G16B16A16Float, RhiResourceDimension::Texture2D);
    ETH_TOOLONLY(schedule.NewRT(ACCESS_GFX_RT(MetadataBuffer), resolution.x, resolution.y, RhiFormat::R32Uint));
    ETH_TOOLONLY(schedule.NewSR(ACCESS_GFX_SR(MetadataBuffer), resolution.x, resolution.y, RhiFormat::R32Uint, RhiResourceDimension::Texture2D));

    schedule.Read(ACCESS_GFX_CB(GlobalConstants));
    schedule.Read(ACCESS_GFX_SR(MaterialTable));
}

void Ether::Graphics::GBufferProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();

    const std::vector<VisualBatch>& batches = GraphicCore::GetGraphicRenderer().GetThreadedRenderData().m_VisualBatches;

    ctx.PushMarker("Clear");
    ctx.ClearColor(*ACCESS_GFX_RT(GBufferTextureA));
    ctx.ClearColor(*ACCESS_GFX_RT(GBufferTextureB));
    ctx.ClearColor(*ACCESS_GFX_RT(GBufferTextureC));
    ctx.ClearDepthStencil(*ACCESS_GFX_DS(SceneDepth));
    ETH_TOOLONLY(ctx.ClearColor(*ACCESS_GFX_RT(MetadataBuffer)));
    ctx.PopMarker();

    ctx.PushMarker("Draw Geometry");
    ctx.SetViewport(gfxDisplay.GetViewport());
    ctx.SetScissorRect(gfxDisplay.GetScissorRect());
    ctx.SetPrimitiveTopology(RhiPrimitiveTopology::TriangleList);
    ctx.SetSrvCbvUavDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
    ctx.SetSamplerDescriptorHeap(GraphicCore::GetSamplerAllocator().GetDescriptorHeap());
    ctx.SetGraphicRootSignature(*m_RootSignature);
    ctx.SetGraphicPipelineState((RhiGraphicPipelineState&)rc.GetPipelineState(*m_PsoDesc));
    ctx.Bind(ACCESS_GFX_CB(GlobalConstants), GetRingBufferOffset());
    ctx.Bind(ACCESS_GFX_SR(MaterialTable));

    std::vector<RhiRenderTargetView> rtvs;
    rtvs.emplace_back(*ACCESS_GFX_RT(GBufferTextureA));
    rtvs.emplace_back(*ACCESS_GFX_RT(GBufferTextureB));
    rtvs.emplace_back(*ACCESS_GFX_RT(GBufferTextureC));
    ETH_TOOLONLY(rtvs.emplace_back(*ACCESS_GFX_RT(MetadataBuffer)));
    ctx.SetRenderTargets(rtvs.data(), rtvs.size(), &(*ACCESS_GFX_DS(SceneDepth)));

    // Batch by material only for now
    for (const VisualBatch& batch : batches)
    {
        ETH_MARKER_EVENT("Material Batch");
        if (batch.m_Material->HasTranslucency())
            continue;

        for (const Visual& visual : batch.m_Visuals)
        {
            if (visual.m_Culled)
                continue;

            auto alloc = GetFrameAllocator().Allocate({ sizeof(Shader::InstanceParams), 256 });
            if (alloc == nullptr)
            {
                LogGraphicsError("Failed to allocate from frame allocator. Visual is discarded.");
                continue;
            }
            Shader::InstanceParams* instanceParams = (Shader::InstanceParams*)alloc->GetCpuHandle();
            instanceParams->m_MaterialIdx = batch.m_Material->GetTransientMaterialIdx();
            instanceParams->m_ModelMatrix = visual.m_ModelMatrix;
            instanceParams->m_ModelMatrixPrev = visual.m_ModelMatrixPrev;
            instanceParams->m_NormalMatrix = visual.m_ModelMatrix.Inversed().Transposed();
            ETH_TOOLONLY(instanceParams->m_EntityID = visual.m_EntityID);
            ctx.Bind("InstanceParams", ((UploadBufferAllocation&)(*alloc)).GetGpuAddress());
            ctx.SetVertexBuffer(visual.m_Mesh->GetVertexBufferView());
            ctx.SetIndexBuffer(visual.m_Mesh->GetIndexBufferView());
            ctx.DrawIndexedInstanced(visual.m_Mesh->GetNumIndices(), 1);
        }
    }

    ctx.PopMarker();
}

bool Ether::Graphics::GBufferProducer::IsEnabled()
{
    // This will break because we have later passes dependent on this pass, and no frame graph is implemented yet
    //if (GraphicCore::GetGraphicRenderer().GetThreadedRenderData().m_Visuals.empty())
    //    return false;

    return true;
}

void Ether::Graphics::GBufferProducer::CreateShaders()
{
    RhiDevice& gfxDevice = GraphicCore::GetDevice();
    m_VertexShader = gfxDevice.CreateShader({ "basepass_vs.hlsl", "VS_Main", RhiShaderType::Vertex });
    m_PixelShader = gfxDevice.CreateShader({ "gbuffer_ps.hlsl", "PS_Main", RhiShaderType::Pixel });

    // Manually compile shader since raytracing PSO caching has not been implemented yet
    m_VertexShader->Compile();
    m_PixelShader->Compile();

    GraphicCore::GetShaderDaemon().RegisterShader(*m_VertexShader);
    GraphicCore::GetShaderDaemon().RegisterShader(*m_PixelShader);
}

void Ether::Graphics::GBufferProducer::CreateRootSignature()
{
    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc({ &m_VertexShader->GetReflection(), &m_PixelShader->GetReflection() });
    rsDesc->SetFlags(RhiRootSignatureFlag::AllowIAInputLayout | RhiRootSignatureFlag::DirectlyIndexed);
    m_RootSignature = rsDesc->Compile((GetName() + " Root Signature").c_str());
}

void Ether::Graphics::GBufferProducer::CreatePipelineState(ResourceContext& rc)
{
    std::vector<RhiFormat> rtvFormats;
    rtvFormats.emplace_back(RhiFormat::R8G8B8A8Unorm);
    rtvFormats.emplace_back(RhiFormat::R16G16B16A16Float);
    rtvFormats.emplace_back(RhiFormat::R16G16B16A16Float);
    ETH_TOOLONLY(rtvFormats.emplace_back(RhiFormat::R32Uint));

    m_PsoDesc = GraphicCore::GetDevice().CreateGraphicPipelineStateDesc();
    m_PsoDesc->SetVertexShader(*m_VertexShader);
    m_PsoDesc->SetPixelShader(*m_PixelShader);
    m_PsoDesc->SetRenderTargetFormats(rtvFormats.data(), rtvFormats.size());
    m_PsoDesc->SetRootSignature(*m_RootSignature);
    m_PsoDesc->SetInputLayout(VertexFormats::BaseVertexFormat::s_InputElementDesc, VertexFormats::BaseVertexFormat::s_NumElements);
    m_PsoDesc->SetDepthTargetFormat(DepthBufferDsvFormat);
    m_PsoDesc->SetDepthStencilState(GraphicCore::GetGraphicCommon().m_DepthStateReadWrite);
    rc.RegisterPipelineState((GetName() + " Pipeline State").c_str(), *m_PsoDesc);
}

