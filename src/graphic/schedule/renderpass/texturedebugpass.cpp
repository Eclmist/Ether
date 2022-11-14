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

#include "texturedebugpass.h"
#include "graphic/rhi/rhicommandlist.h"
#include "graphic/rhi/rhicommandqueue.h"
#include "graphic/rhi/rhipipelinestate.h"
#include "graphic/rhi/rhirootparameter.h"
#include "graphic/rhi/rhirootsignature.h"

ETH_NAMESPACE_BEGIN

DEFINE_GFX_PASS(TextureDebugPass);

DECLARE_GFX_RESOURCE(GBufferAlbedoTexture);
DECLARE_GFX_RESOURCE(GBufferSpecularTexture);
DECLARE_GFX_RESOURCE(GBufferNormalTexture);
DECLARE_GFX_RESOURCE(GBufferPositionTexture);
DECLARE_GFX_RESOURCE(EntityPickerTexture);

DECLARE_GFX_SRV(GBufferAlbedoTexture);
DECLARE_GFX_SRV(GBufferSpecularTexture);
DECLARE_GFX_SRV(GBufferNormalTexture);
DECLARE_GFX_SRV(GBufferPositionTexture);
DEFINE_GFX_SRV(EntityPickerTexture);

TextureDebugPass::TextureDebugPass()
    : RenderPass("Texture Debug Pass")
{
}

void TextureDebugPass::Initialize()
{
    InitializeShaders();
    InitializeRootSignature();
    InitializePipelineState();
}

void TextureDebugPass::RegisterInputOutput(GraphicContext& context, ResourceContext& rc)
{
    rc.CreateShaderResourceView(GFX_RESOURCE(EntityPickerTexture), GFX_SRV(EntityPickerTexture));
}

void TextureDebugPass::Render(GraphicContext& context, ResourceContext& rc)
{
    EngineConfig& config = EngineCore::GetEngineConfig();
    if (config.m_DebugTextureIndex == 0)
        return;

    uint32_t debugTextureBindlessIdx;

    switch (config.m_DebugTextureIndex)
    {
    case 1:
        debugTextureBindlessIdx = GraphicCore::GetBindlessResourceManager().GetViewIndex(GFX_SRV(GBufferAlbedoTexture));
        break;
    case 2:
        debugTextureBindlessIdx = GraphicCore::GetBindlessResourceManager().GetViewIndex(GFX_SRV(GBufferSpecularTexture));
        break;
    case 3:
        debugTextureBindlessIdx = GraphicCore::GetBindlessResourceManager().GetViewIndex(GFX_SRV(GBufferNormalTexture));
        break;
    case 4:
        debugTextureBindlessIdx = GraphicCore::GetBindlessResourceManager().GetViewIndex(GFX_SRV(GBufferPositionTexture));
        break;
    case 5:
        debugTextureBindlessIdx = GraphicCore::GetBindlessResourceManager().GetViewIndex(GFX_SRV(EntityPickerTexture));
        break;
    default:
        return;
    }

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
    context.GetCommandList()->SetGraphicRootSignature(m_RootSignature);
    context.GetCommandList()->SetPrimitiveTopology(RhiPrimitiveTopology::TriangleStrip);
    context.GetCommandList()->SetDescriptorHeaps({ 1, &GraphicCore::GetGpuDescriptorAllocator().GetDescriptorHeap() });
    context.GetCommandList()->SetRootConstants({ 0, 1, 0, &debugTextureBindlessIdx });
    context.GetCommandList()->DrawInstanced({ 4, 1, 0, 0 });

    context.FinalizeAndExecute();
    context.Reset();
}

void TextureDebugPass::InitializeShaders()
{
    m_VertexShader = std::make_unique<Shader>(L"toolmode\\texturedebug.hlsl", L"VS_Main", L"vs_6_6", ShaderType::Vertex);
    m_PixelShader = std::make_unique<Shader>(L"toolmode\\texturedebug.hlsl", L"PS_Main", L"ps_6_6", ShaderType::Pixel);

    m_VertexShader->Compile();
    m_PixelShader->Compile();
    m_VertexShader->SetRecompiled(false);
    m_PixelShader->SetRecompiled(false);
}

void TextureDebugPass::InitializePipelineState()
{
    m_PipelineState.SetName(L"TextureDebugPass::PipelineState");

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
    creationPSO.SetRootSignature(m_RootSignature);
    creationPSO.Finalize(m_PipelineState);

}

void TextureDebugPass::InitializeRootSignature()
{
    m_RootSignature.SetName(L"TextureDebugPass::RootSignature");
    RhiRootSignature tempRS(1, 1);
    tempRS.GetSampler(0) = GraphicCore::GetGraphicCommon().m_PointSampler;
    tempRS[0]->SetAsConstant({ 1, 0, RhiShaderVisibility::All, 1 });
    tempRS.Finalize(GraphicCore::GetGraphicCommon().m_BindlessRootSignatureFlags, m_RootSignature);
}

ETH_NAMESPACE_END

