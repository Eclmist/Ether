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

#include "debugpass.h"
#include "graphic/rhi/rhicommandlist.h"
#include "graphic/rhi/rhicommandqueue.h"
#include "graphic/rhi/rhipipelinestate.h"
#include "graphic/rhi/rhirootparameter.h"
#include "graphic/rhi/rhirootsignature.h"

ETH_NAMESPACE_BEGIN

DEFINE_GFX_PASS(DebugPass);

DEFINE_GFX_RESOURCE(DebugOutputTexture);

DebugPass::DebugPass()
    : RenderPass("Debug Pass")
{
}

void DebugPass::Initialize()
{
    InitializeShaders();
    InitializePipelineState();
}

void DebugPass::RegisterInputOutput(GraphicContext& context, ResourceContext& rc)
{
}

void DebugPass::Render(GraphicContext& context, ResourceContext& rc)
{
    EngineConfig& config = EngineCore::GetEngineConfig();

    // TODO: Properly support shader hot reload - each PSO should check their own shaders
    if (m_VertexShader->HasRecompiled() || m_PixelShader->HasRecompiled())
    {
        context.GetCommandQueue()->Flush();
        InitializePipelineState();
        m_VertexShader->SetRecompiled(false);
        m_PixelShader->SetRecompiled(false);
    }

    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();

    context.SetRenderTarget(gfxDisplay.GetCurrentBackBufferRtv());
    context.SetViewport(gfxDisplay.GetViewport());
    context.SetScissor(gfxDisplay.GetScissorRect());

    context.GetCommandList()->SetPipelineState(m_PipelineState);
    context.GetCommandList()->SetPrimitiveTopology(RhiPrimitiveTopology::TriangleStrip);
    context.GetCommandList()->SetDescriptorHeaps({ 1, &GraphicCore::GetGpuDescriptorAllocator().GetDescriptorHeap() });
    context.GetCommandList()->SetGraphicRootSignature(GraphicCore::GetGraphicCommon().m_BindlessRootSignature);
    context.GetCommandList()->DrawInstanced({ 4, 1, 0, 0 });

    context.FinalizeAndExecute();
    context.Reset();
}

void DebugPass::InitializeShaders()
{
    m_VertexShader = std::make_unique<Shader>(L"debug.hlsl", L"VS_Main", L"vs_6_6", ShaderType::Vertex);
    m_PixelShader = std::make_unique<Shader>(L"debug.hlsl", L"PS_Main", L"ps_6_6", ShaderType::Pixel);

    m_VertexShader->Compile();
    m_PixelShader->Compile();
    m_VertexShader->SetRecompiled(false);
    m_PixelShader->SetRecompiled(false);
}

void DebugPass::InitializePipelineState()
{
    m_PipelineState.SetName(L"DebugPass::PipelineState");

    RhiPipelineState creationPSO;
    creationPSO.SetBlendState(GraphicCore::GetGraphicCommon().m_BlendDisabled);
    creationPSO.SetRasterizerState(GraphicCore::GetGraphicCommon().m_RasterizerDefault);
    creationPSO.SetPrimitiveTopology(RhiPrimitiveTopologyType::Triangle);
    creationPSO.SetVertexShader(m_VertexShader->GetCompiledShader(), m_VertexShader->GetCompiledShaderSize());
    creationPSO.SetPixelShader(m_PixelShader->GetCompiledShader(), m_PixelShader->GetCompiledShaderSize());
    creationPSO.SetInputLayout(GraphicCore::GetGraphicCommon().m_DefaultInputLayout);
    creationPSO.SetRenderTargetFormat(BackBufferFormat);
    creationPSO.SetDepthStencilState(GraphicCore::GetGraphicCommon().m_DepthStateDisabled);
    creationPSO.SetSamplingDesc(1, 0);
    creationPSO.SetRootSignature(GraphicCore::GetGraphicCommon().m_BindlessRootSignature);
    creationPSO.Finalize(m_PipelineState);
}

ETH_NAMESPACE_END

