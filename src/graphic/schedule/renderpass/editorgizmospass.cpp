/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "editorgizmospass.h"
#include "graphic/rhi/rhicommandlist.h"
#include "graphic/rhi/rhicommandqueue.h"
#include "graphic/rhi/rhipipelinestate.h"
#include "graphic/rhi/rhirootparameter.h"
#include "graphic/rhi/rhirootsignature.h"

ETH_NAMESPACE_BEGIN

DEFINE_GFX_PASS(EditorGizmosPass);

DEFINE_GFX_RESOURCE(EditorGizmosTexture);
DECLARE_GFX_RESOURCE(GlobalCommonConstants);

DEFINE_GFX_RTV(EditorGizmosTexture);
DECLARE_GFX_DSV(DepthStencilTexture);
DECLARE_GFX_SRV(GBufferPositionTexture);

EditorGizmosPass::EditorGizmosPass()
    : RenderPass("Editor Gizmos Pass")
{
}

void EditorGizmosPass::Initialize()
{
    InitializeShaders();
    InitializeRasterizerDesc();
    InitializeRootSignature();
    InitializePipelineState();
}

void EditorGizmosPass::RegisterInputOutput(GraphicContext& context, ResourceContext& rc)
{
    RhiViewportDesc vp = context.GetViewport();
    rc.CreateTexture2DResource(vp.m_Width, vp.m_Height, RhiFormat::R8G8B8A8Unorm, GFX_RESOURCE(EditorGizmosTexture));
    rc.CreateRenderTargetView(GFX_RESOURCE(EditorGizmosTexture), GFX_RTV(EditorGizmosTexture));
}

void EditorGizmosPass::Render(GraphicContext& context, ResourceContext& rc)
{
    // TODO: Properly support shader hot reload - each PSO should check their own shaders
    if (m_VertexShader->HasRecompiled() || m_PixelShader->HasRecompiled())
    {
        context.GetCommandQueue()->Flush();
        InitializePipelineState();
        m_VertexShader->SetRecompiled(false);
        m_PixelShader->SetRecompiled(false);
    }

    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();

    context.SetRenderTargets(1, &gfxDisplay.GetCurrentBackBufferRTV(), GFX_DSV(DepthStencilTexture));
    context.SetViewport(gfxDisplay.GetViewport());
    context.SetScissor(gfxDisplay.GetScissorRect());

    context.GetCommandList()->SetPipelineState(m_PipelineState);
    context.GetCommandList()->SetGraphicRootSignature(m_RootSignature);
    context.GetCommandList()->SetPrimitiveTopology(RhiPrimitiveTopology::TriangleStrip);
    context.GetCommandList()->SetDescriptorHeaps({ 1, &GraphicCore::GetSRVDescriptorHeap() });
    context.GetCommandList()->SetRootConstantBuffer({ 0, GFX_RESOURCE(GlobalCommonConstants) });
    context.GetCommandList()->SetRootDescriptorTable({ 1, GFX_SRV(GBufferPositionTexture) });
    context.GetCommandList()->DrawInstanced({ 4, 1, 0, 0 });

    context.FinalizeAndExecute();
    context.Reset();
}

void EditorGizmosPass::InitializeShaders()
{
    m_VertexShader = std::make_unique<Shader>(L"toolmode\\editorgizmos.hlsl", L"VS_Main", L"vs_6_0", ShaderType::Vertex);
    m_PixelShader = std::make_unique<Shader>(L"toolmode\\editorgizmos.hlsl", L"PS_Main", L"ps_6_0", ShaderType::Pixel);

    m_VertexShader->Compile();
    m_PixelShader->Compile();
    m_VertexShader->SetRecompiled(false);
    m_PixelShader->SetRecompiled(false);
}

void EditorGizmosPass::InitializePipelineState()
{
    m_PipelineState.SetName(L"EditorGizmosPass::PipelineState");

    RhiPipelineState creationPSO;
    creationPSO.SetBlendState(GraphicCore::GetGraphicCommon().m_BlendTraditional);
    creationPSO.SetRasterizerState(m_RasterizerDesc);
    creationPSO.SetPrimitiveTopology(RhiPrimitiveTopologyType::Triangle);
    creationPSO.SetVertexShader(m_VertexShader->GetCompiledShader(), m_VertexShader->GetCompiledShaderSize());
    creationPSO.SetPixelShader(m_PixelShader->GetCompiledShader(), m_PixelShader->GetCompiledShaderSize());
    creationPSO.SetInputLayout(GraphicCore::GetGraphicCommon().m_DefaultInputLayout);
    creationPSO.SetRenderTargetFormat(BackBufferFormat);
    creationPSO.SetDepthTargetFormat(RhiFormat::D24UnormS8Uint);
    creationPSO.SetDepthStencilState(GraphicCore::GetGraphicCommon().m_DepthStateReadOnly);
    creationPSO.SetSamplingDesc(1, 0);
    creationPSO.SetRootSignature(m_RootSignature);
    creationPSO.Finalize(m_PipelineState);
}

void EditorGizmosPass::InitializeRasterizerDesc()
{
    m_RasterizerDesc = GraphicCore::GetGraphicCommon().m_RasterizerDefault;
    m_RasterizerDesc.m_CullMode = RhiCullMode::None;
}

void EditorGizmosPass::InitializeRootSignature()
{
    m_RootSignature.SetName(L"EditorGizmosPass::RootSignature");

    RhiRootSignature tempRS(2, 1);
    tempRS.GetSampler(0) = GraphicCore::GetGraphicCommon().m_PointSampler;
    tempRS[0]->SetAsConstantBufferView({ 0, 0, RhiShaderVisibility::All });
    tempRS[1]->SetAsDescriptorRange({ 0, 0, RhiShaderVisibility::Pixel, RhiDescriptorRangeType::SRV, 1 });
    tempRS.Finalize(GraphicCore::GetGraphicCommon().m_DefaultRootSignatureFlags, m_RootSignature);
}

ETH_NAMESPACE_END

