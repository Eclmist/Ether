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

#include "texturedebugpass.h"
#include "graphic/rhi/rhicommandlist.h"
#include "graphic/rhi/rhicommandqueue.h"
#include "graphic/rhi/rhipipelinestate.h"
#include "graphic/rhi/rhirootparameter.h"
#include "graphic/rhi/rhirootsignature.h"

ETH_NAMESPACE_BEGIN

DEFINE_GFX_PASS(TextureDebugPass);

DECLARE_GFX_RESOURCE(GBufferAlbedoTexture);
DECLARE_GFX_RESOURCE(GBufferNormalTexture);
DECLARE_GFX_RESOURCE(GBufferPosDepthTexture);

DECLARE_GFX_SRV(GBufferAlbedoTexture);
DECLARE_GFX_SRV(GBufferNormalTexture);
DECLARE_GFX_SRV(GBufferPosDepthTexture);

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
    rc.CreateShaderResourceView(GFX_RESOURCE(GBufferAlbedoTexture), GFX_SRV(GBufferAlbedoTexture));
    rc.CreateShaderResourceView(GFX_RESOURCE(GBufferNormalTexture), GFX_SRV(GBufferNormalTexture));
    rc.CreateShaderResourceView(GFX_RESOURCE(GBufferPosDepthTexture), GFX_SRV(GBufferPosDepthTexture));
}

void TextureDebugPass::Render(GraphicContext& context, ResourceContext& rc)
{
    EngineConfig& config = EngineCore::GetEngineConfig();
    if (config.m_DebugTextureIndex == 0)
        return;

    OPTICK_EVENT("Texture Debug Pass - Render");

    // TODO: Properly support shader hot reload - each PSO should check their own shaders
    if (m_VertexShader->HasRecompiled() || m_PixelShader->HasRecompiled())
    {
        context.GetCommandQueue()->Flush();
        InitializePipelineState();
        m_VertexShader->SetRecompiled(false);
        m_PixelShader->SetRecompiled(false);
    }

    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();

    context.SetRenderTarget(gfxDisplay.GetCurrentBackBufferRTV());
    context.SetViewport(gfxDisplay.GetViewport());
    context.SetScissor(gfxDisplay.GetScissorRect());

    context.GetCommandList()->SetPipelineState(m_PipelineState);
    context.GetCommandList()->SetGraphicRootSignature(m_RootSignature);
    context.GetCommandList()->SetPrimitiveTopology(RHIPrimitiveTopology::TriangleStrip);
    context.GetCommandList()->SetDescriptorHeaps({ 1, &GraphicCore::GetSRVDescriptorHeap() });
    context.GetCommandList()->SetRootConstants({ 0, 1, 0, &config.m_DebugTextureIndex });
    // TODO: Setup bindless textures
    context.GetCommandList()->SetRootDescriptorTable({ 1, GFX_SRV(GBufferAlbedoTexture) });
    context.GetCommandList()->SetRootDescriptorTable({ 2, GFX_SRV(GBufferNormalTexture) });
    context.GetCommandList()->SetRootDescriptorTable({ 3, GFX_SRV(GBufferPosDepthTexture) });
    context.GetCommandList()->DrawInstanced({ 4, 1, 0, 0 });

    context.FinalizeAndExecute();
    context.Reset();
}

void TextureDebugPass::InitializeShaders()
{
    m_VertexShader = std::make_unique<Shader>(L"debug\\texturedebug.hlsl", L"VS_Main", L"vs_6_0", ShaderType::Vertex);
    m_PixelShader = std::make_unique<Shader>(L"debug\\texturedebug.hlsl", L"PS_Main", L"ps_6_0", ShaderType::Pixel);

    m_VertexShader->Compile();
    m_PixelShader->Compile();
    m_VertexShader->SetRecompiled(false);
    m_PixelShader->SetRecompiled(false);
}

void TextureDebugPass::InitializePipelineState()
{
    m_PipelineState.SetName(L"TextureDebugPass::PipelineState");

    RHIPipelineState creationPSO;
    creationPSO.SetBlendState(GraphicCore::GetGraphicCommon().m_BlendDisabled);
    creationPSO.SetRasterizerState(GraphicCore::GetGraphicCommon().m_RasterizerDefault);
    creationPSO.SetPrimitiveTopology(RHIPrimitiveTopologyType::Triangle);
    creationPSO.SetVertexShader(m_VertexShader->GetCompiledShader(), m_VertexShader->GetCompiledShaderSize());
    creationPSO.SetPixelShader(m_PixelShader->GetCompiledShader(), m_PixelShader->GetCompiledShaderSize());
    creationPSO.SetInputLayout(GraphicCore::GetGraphicCommon().m_DefaultInputLayout);
    creationPSO.SetRenderTargetFormat(RHIFormat::R8G8B8A8Unorm);
    creationPSO.SetDepthStencilState(GraphicCore::GetGraphicCommon().m_DepthStateDisabled);
    creationPSO.SetSamplingDesc(1, 0);
    creationPSO.SetRootSignature(m_RootSignature);
    creationPSO.Finalize(m_PipelineState);

}

void TextureDebugPass::InitializeRootSignature()
{
    m_RootSignature.SetName(L"TextureDebugPass::RootSignature");

    RHIRootSignature tempRS(4, 1);
    tempRS.GetSampler(0) = GraphicCore::GetGraphicCommon().m_PointSampler;
    tempRS[0]->SetAsConstant({ 0, 0, RHIShaderVisibility::All, 32 });
    tempRS[1]->SetAsDescriptorRange({ 0, 0, RHIShaderVisibility::Pixel, RHIDescriptorRangeType::SRV, 1 });
    tempRS[2]->SetAsDescriptorRange({ 1, 0, RHIShaderVisibility::Pixel, RHIDescriptorRangeType::SRV, 1 });
    tempRS[3]->SetAsDescriptorRange({ 2, 0, RHIShaderVisibility::Pixel, RHIDescriptorRangeType::SRV, 1 });
    tempRS.Finalize(GraphicCore::GetGraphicCommon().m_DefaultRootSignatureFlags, m_RootSignature);
}

ETH_NAMESPACE_END

