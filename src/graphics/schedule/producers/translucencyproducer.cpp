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

#include "translucencyproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/config/graphicconfig.h"
#include "graphics/resources/staticmesh.h"
#include "graphics/resources/skinnedmesh.h"
#include "graphics/resources/material.h"

#include "graphics/shaders/common/globalconstants.h"
#include "graphics/shaders/common/instanceparams.h"
#include "graphics/shaders/common/irradiancefieldparams.h"

DEFINE_GFX_PA(TranslucencyProducer)

DECLARE_GFX_RT(SceneColor)
DECLARE_GFX_DS(SceneDepth)
DECLARE_GFX_SR(SceneDepth)
DECLARE_GFX_CB(GlobalConstants)
DECLARE_GFX_SR(MaterialTable)
DECLARE_GFX_SR(IrradianceFieldIrradianceAtlas)
DECLARE_GFX_SR(IrradianceFieldDepthAtlas)
DECLARE_GFX_CB(IrradianceFieldParams)

#if ETH_TOOLMODE
DECLARE_GFX_RT(MetadataBuffer)
#endif


Ether::Graphics::TranslucencyProducer::TranslucencyProducer()
    : GraphicProducer("TranslucencyProducer")
{
}

void Ether::Graphics::TranslucencyProducer::Initialize(ResourceContext& rc)
{
    CreateShaders();
    CreateRootSignature();
    CreatePipelineState(rc);
}

void Ether::Graphics::TranslucencyProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();
    schedule.Read(ACCESS_GFX_RT(SceneColor));
    schedule.Read(ACCESS_GFX_DS(SceneDepth));
    schedule.Read(ACCESS_GFX_SR(SceneDepth));
    schedule.Read(ACCESS_GFX_CB(GlobalConstants));
    schedule.Read(ACCESS_GFX_SR(MaterialTable));
    schedule.Read(ACCESS_GFX_SR(IrradianceFieldIrradianceAtlas));
    schedule.Read(ACCESS_GFX_SR(IrradianceFieldDepthAtlas));
    schedule.Read(ACCESS_GFX_CB(IrradianceFieldParams));

#if ETH_TOOLMODE
    if (config.IsTranslucencyPickingEnabled())
    {
        schedule.Read(ACCESS_GFX_RT(MetadataBuffer));
    }
#endif
}

void Ether::Graphics::TranslucencyProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    ETH_MARKER_EVENT("TranslucencyProducer");

    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();

    const std::vector<VisualBatch>& batches = GraphicCore::GetGraphicRenderer().GetThreadedRenderData().m_VisualBatches;

    ctx.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::RenderTarget);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_RT(SceneColor)), RhiResourceState::RenderTarget);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_DS(SceneDepth)), RhiResourceState::DepthRead);

    ctx.PushMarker("Draw Translucencies");
    ctx.SetViewport(gfxDisplay.GetViewport());
    ctx.SetScissorRect(gfxDisplay.GetScissorRect());
    ctx.SetPrimitiveTopology(RhiPrimitiveTopology::TriangleList);
    ctx.SetSrvCbvUavDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
    ctx.SetSamplerDescriptorHeap(GraphicCore::GetSamplerAllocator().GetDescriptorHeap());
    ctx.SetGraphicRootSignature(*m_RootSignature);
    ctx.SetGraphicPipelineState((RhiGraphicPipelineState&)rc.GetPipelineState(*m_PsoDesc));
    ctx.SetRenderTarget(*ACCESS_GFX_RT(SceneColor), &(*ACCESS_GFX_DS(SceneDepth)));
    ctx.Bind(ACCESS_GFX_CB(GlobalConstants), GetRingBufferOffset());
    ctx.Bind(ACCESS_GFX_SR(MaterialTable));
    ctx.Bind(ACCESS_GFX_SR(SceneDepth));
    ctx.Bind(ACCESS_GFX_SR(IrradianceFieldIrradianceAtlas));
    ctx.Bind(ACCESS_GFX_SR(IrradianceFieldDepthAtlas));
    ctx.Bind(ACCESS_GFX_CB(IrradianceFieldParams), AlignUp(sizeof(Shader::IrradianceFieldParams), 256) * GraphicCore::GetGraphicDisplay().GetBackBufferIndex());

#if ETH_TOOLMODE
    if (config.IsTranslucencyPickingEnabled())
    {
        ctx.SetGraphicPipelineState((RhiGraphicPipelineState&)rc.GetPipelineState(*m_MetadataWritePsoDesc));
        std::vector<RhiRenderTargetView> rtvs;
        rtvs.emplace_back(*ACCESS_GFX_RT(SceneColor));
        rtvs.emplace_back(*ACCESS_GFX_RT(MetadataBuffer));
        ctx.SetRenderTargets(rtvs.data(), rtvs.size(), &(*ACCESS_GFX_DS(SceneDepth)));
    }
#endif

    // Batch by material only for now
    for (const VisualBatch& batch : batches)
    {
        if (!batch.m_Material->HasTranslucency())
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

bool Ether::Graphics::TranslucencyProducer::IsEnabled()
{
    if (GraphicCore::GetGraphicRenderer().GetThreadedRenderData().m_Visuals.empty())
        return false;

    if (!GraphicCore::GetGraphicConfig().m_TranslucencyEnabled)
        return false;

    return true;
}

void Ether::Graphics::TranslucencyProducer::CreateShaders()
{
    RhiDevice& gfxDevice = GraphicCore::GetDevice();
    m_VertexShader = gfxDevice.CreateShader({ "basepass_vs.hlsl", "VS_Main", RhiShaderType::Vertex });
    m_PixelShader = gfxDevice.CreateShader({ "translucency\\forwardtranslucency_ps.hlsl", "PS_Main", RhiShaderType::Pixel });
    m_VertexShader->Compile();
    m_PixelShader->Compile();

    GraphicCore::GetShaderDaemon().RegisterShader(*m_VertexShader);
    GraphicCore::GetShaderDaemon().RegisterShader(*m_PixelShader);

#if ETH_TOOLMODE
    m_MetadataPS = gfxDevice.CreateShader({ "translucency\\forwardtranslucency_ps.hlsl", "PS_Main", RhiShaderType::Pixel });
    m_MetadataPS->AddDefinition("WRITE_METADATA");
    m_MetadataPS->Compile();
    GraphicCore::GetShaderDaemon().RegisterShader(*m_MetadataPS);
#endif
}

void Ether::Graphics::TranslucencyProducer::CreateRootSignature()
{
    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc({ &m_VertexShader->GetReflection(), &m_PixelShader->GetReflection() });
    rsDesc->SetFlags(RhiRootSignatureFlag::AllowIAInputLayout | RhiRootSignatureFlag::DirectlyIndexed);
    m_RootSignature = rsDesc->Compile((GetName() + " Root Signature").c_str());
}

void Ether::Graphics::TranslucencyProducer::CreatePipelineState(ResourceContext& rc)
{
    m_PsoDesc = GraphicCore::GetDevice().CreateGraphicPipelineStateDesc();
    m_PsoDesc->SetVertexShader(*m_VertexShader);
    m_PsoDesc->SetPixelShader(*m_PixelShader);
    m_PsoDesc->SetRenderTargetFormat(BackBufferHdrFormat);
    m_PsoDesc->SetRootSignature(*m_RootSignature);
    m_PsoDesc->SetInputLayout(VertexFormats::BaseVertexFormat::s_InputElementDesc, VertexFormats::BaseVertexFormat::s_NumElements);
    m_PsoDesc->SetDepthTargetFormat(DepthBufferDsvFormat);
    m_PsoDesc->SetDepthStencilState(GraphicCore::GetGraphicCommon().m_DepthStateReadOnly);
    m_PsoDesc->SetBlendState(GraphicCore::GetGraphicCommon().m_BlendTraditional);
    rc.RegisterPipelineState((GetName() + " Pipeline State").c_str(), *m_PsoDesc);

#if ETH_TOOLMODE
    std::vector<RhiFormat> rtvFormats;
    rtvFormats.emplace_back(BackBufferHdrFormat);
    rtvFormats.emplace_back(RhiFormat::R32Uint);

    m_MetadataWritePsoDesc = GraphicCore::GetDevice().CreateGraphicPipelineStateDesc();
    m_MetadataWritePsoDesc->SetVertexShader(*m_VertexShader);
    m_MetadataWritePsoDesc->SetPixelShader(*m_MetadataPS);
    m_MetadataWritePsoDesc->SetRenderTargetFormats(rtvFormats.data(), rtvFormats.size());
    m_MetadataWritePsoDesc->SetRootSignature(*m_RootSignature);
    m_MetadataWritePsoDesc->SetInputLayout(VertexFormats::BaseVertexFormat::s_InputElementDesc, VertexFormats::BaseVertexFormat::s_NumElements);
    m_MetadataWritePsoDesc->SetDepthTargetFormat(DepthBufferDsvFormat);
    m_MetadataWritePsoDesc->SetDepthStencilState(GraphicCore::GetGraphicCommon().m_DepthStateReadOnly);
    m_MetadataWritePsoDesc->SetBlendState(GraphicCore::GetGraphicCommon().m_BlendTraditional, 0);
    m_MetadataWritePsoDesc->SetBlendState(GraphicCore::GetGraphicCommon().m_BlendDisabled, 1);
    rc.RegisterPipelineState((GetName() + " Pipeline State").c_str(), *m_MetadataWritePsoDesc);
#endif
}

