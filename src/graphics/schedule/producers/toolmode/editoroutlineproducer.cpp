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

#if ETH_TOOLMODE

#include "editoroutlineproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/config/graphicconfig.h"
#include "graphics/resources/staticmesh.h"
#include "graphics/resources/skinnedmesh.h"
#include "graphics/resources/material.h"

#include "graphics/shaders/common/globalconstants.h"
#include "graphics/shaders/common/instanceparams.h"

DEFINE_GFX_PA(EditorOutlineProducer)
DEFINE_GFX_RT(OutlineMaskTexture)
DEFINE_GFX_SR(OutlineMaskTexture)

DECLARE_GFX_CB(GlobalConstants)

Ether::Graphics::EditorOutlineProducer::EditorOutlineProducer()
    : GraphicProducer("EditorOutlineProducer")
{
}

void Ether::Graphics::EditorOutlineProducer::Initialize(ResourceContext& rc)
{
    CreateShaders();
    CreateRootSignature();
    CreatePipelineState(rc);
}

void Ether::Graphics::EditorOutlineProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();

    schedule.NewRT(ACCESS_GFX_RT(OutlineMaskTexture), resolution.x, resolution.y, BackBufferLdrFormat);
    schedule.NewSR(ACCESS_GFX_SR(OutlineMaskTexture), resolution.x, resolution.y, BackBufferLdrFormat, RhiResourceDimension::Texture2D);
    schedule.Read(ACCESS_GFX_CB(GlobalConstants));
}

void Ether::Graphics::EditorOutlineProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();

    const std::vector<Visual>& visuals = GraphicCore::GetGraphicRenderer().GetThreadedRenderData().m_Visuals;

    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_RT(OutlineMaskTexture)), RhiResourceState::RenderTarget);

    ctx.PushMarker("Draw Outlined Geometry");
    ctx.SetViewport(gfxDisplay.GetViewport());
    ctx.SetScissorRect(gfxDisplay.GetScissorRect());
    ctx.SetPrimitiveTopology(RhiPrimitiveTopology::TriangleList);
    ctx.SetSrvCbvUavDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
    ctx.SetSamplerDescriptorHeap(GraphicCore::GetSamplerAllocator().GetDescriptorHeap());
    ctx.SetGraphicRootSignature(*m_RootSignature);
    ctx.SetGraphicPipelineState((RhiGraphicPipelineState&)rc.GetPipelineState(*m_PsoDesc));
    ctx.Bind(ACCESS_GFX_CB(GlobalConstants), GetRingBufferOffset());
    ctx.SetRenderTarget(GraphicCore::GetGraphicDisplay().GetBackBufferRtv());

	for (const Visual& visual : visuals)
	{
		if (visual.m_Culled)
			continue;
        if (!visual.m_ToolmodeSelected)
			continue;

        auto alloc = GetFrameAllocator().Allocate({ sizeof(Shader::InstanceParams), 256 });
        Shader::InstanceParams* instanceParams = (Shader::InstanceParams*)alloc->GetCpuHandle();
        instanceParams->m_MaterialIdx = visual.m_Material->GetTransientMaterialIdx();
        instanceParams->m_ModelMatrix = visual.m_ModelMatrix;
        instanceParams->m_ModelMatrixPrev = visual.m_ModelMatrixPrev;
        instanceParams->m_NormalMatrix = visual.m_ModelMatrix.Inversed().Transposed();
        ctx.Bind("InstanceParams", ((UploadBufferAllocation&)(*alloc)).GetGpuAddress());
        ctx.SetVertexBuffer(visual.m_Mesh->GetVertexBufferView());
		ctx.SetIndexBuffer(visual.m_Mesh->GetIndexBufferView());
		ctx.DrawIndexedInstanced(visual.m_Mesh->GetNumIndices(), 1);
	}

    ctx.PopMarker();
}

bool Ether::Graphics::EditorOutlineProducer::IsEnabled()
{
    if (GraphicCore::GetGraphicRenderer().GetThreadedRenderData().m_Visuals.empty())
        return false;

    // TODO: Add check for whether or not outline drawings are enabled in toolmode

    return true;
}

void Ether::Graphics::EditorOutlineProducer::CreateShaders()
{
    RhiDevice& gfxDevice = GraphicCore::GetDevice();
    m_VertexShader = gfxDevice.CreateShader({ "basepass_vs.hlsl", "VS_Main", RhiShaderType::Vertex });
    m_PixelShader = gfxDevice.CreateShader({ "toolmode\\editoroutline_ps.hlsl", "PS_Main", RhiShaderType::Pixel });

    // Manually compile shader since raytracing PSO caching has not been implemented yet
    m_VertexShader->Compile();
    m_PixelShader->Compile();

    GraphicCore::GetShaderDaemon().RegisterShader(*m_VertexShader);
    GraphicCore::GetShaderDaemon().RegisterShader(*m_PixelShader);
}

void Ether::Graphics::EditorOutlineProducer::CreateRootSignature()
{
    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc({ &m_VertexShader->GetReflection(), &m_PixelShader->GetReflection() });
    rsDesc->SetFlags(RhiRootSignatureFlag::AllowIAInputLayout | RhiRootSignatureFlag::DirectlyIndexed);
    m_RootSignature = rsDesc->Compile((GetName() + " Root Signature").c_str());
}

void Ether::Graphics::EditorOutlineProducer::CreatePipelineState(ResourceContext& rc)
{
    m_PsoDesc = GraphicCore::GetDevice().CreateGraphicPipelineStateDesc();
    m_PsoDesc->SetVertexShader(*m_VertexShader);
    m_PsoDesc->SetPixelShader(*m_PixelShader);
    m_PsoDesc->SetRenderTargetFormat(BackBufferLdrFormat);
    m_PsoDesc->SetRootSignature(*m_RootSignature);
    m_PsoDesc->SetInputLayout(VertexFormats::BaseVertexFormat::s_InputElementDesc, VertexFormats::BaseVertexFormat::s_NumElements);
    m_PsoDesc->SetDepthStencilState(GraphicCore::GetGraphicCommon().m_DepthStateDisabled);
    m_PsoDesc->SetRasterizerState(GraphicCore::GetGraphicCommon().m_RasterizerWireframe);
    rc.RegisterPipelineState((GetName() + " Pipeline State").c_str(), *m_PsoDesc);
}

#endif
