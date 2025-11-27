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

DEFINE_GFX_PA(TranslucencyProducer)

DECLARE_GFX_RT(SceneColor)
DECLARE_GFX_DS(SceneDepth)
DECLARE_GFX_SR(SceneDepth)
DECLARE_GFX_CB(GlobalConstants)
DECLARE_GFX_SR(MaterialTable)

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
    schedule.Read(ACCESS_GFX_RT(SceneColor));
    schedule.Read(ACCESS_GFX_DS(SceneDepth));
    schedule.Read(ACCESS_GFX_SR(SceneDepth));
    schedule.Read(ACCESS_GFX_CB(GlobalConstants));
    schedule.Read(ACCESS_GFX_SR(MaterialTable));
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
    ctx.Bind(ACCESS_GFX_CB(GlobalConstants), GetRingBufferOffset());
    ctx.Bind(ACCESS_GFX_SR(MaterialTable));
    ctx.Bind(ACCESS_GFX_SR(SceneDepth));
    ctx.SetRenderTarget(*ACCESS_GFX_RT(SceneColor), &(*ACCESS_GFX_DS(SceneDepth)));

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
            Shader::InstanceParams* instanceParams = (Shader::InstanceParams*)alloc->GetCpuHandle();
            instanceParams->m_MaterialIdx = batch.m_Material->GetTransientMaterialIdx();
            instanceParams->m_ModelMatrix = visual.m_ModelMatrix;
            instanceParams->m_ModelMatrixPrev = visual.m_ModelMatrixPrev;
            instanceParams->m_NormalMatrix = visual.m_ModelMatrix.Inversed().Transposed();
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

    // Manually compile shader since raytracing PSO caching has not been implemented yet
    m_VertexShader->Compile();
    m_PixelShader->Compile();

    GraphicCore::GetShaderDaemon().RegisterShader(*m_VertexShader);
    GraphicCore::GetShaderDaemon().RegisterShader(*m_PixelShader);
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
}

