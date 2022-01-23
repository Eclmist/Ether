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

#include "gbufferpass.h"
#include "graphic/rhi/rhicommandlist.h"
#include "graphic/rhi/rhicommandqueue.h"
#include "graphic/rhi/rhidevice.h"
#include "graphic/rhi/rhipipelinestate.h"
#include "graphic/rhi/rhiresource.h"
#include "graphic/rhi/rhirootsignature.h"
#include "graphic/rhi/rhirootparameter.h"

ETH_NAMESPACE_BEGIN

DEFINE_GFX_PASS(GBufferPass);

DEFINE_GFX_RESOURCE(GBufferAlbedoTexture);
DEFINE_GFX_RESOURCE(GBufferNormalTexture);
DEFINE_GFX_RESOURCE(GBufferPosDepthTexture);
DEFINE_GFX_RESOURCE(GBufferDepthTexture);

DEFINE_GFX_RTV(GBufferAlbedoTexture);
DEFINE_GFX_RTV(GBufferNormalTexture);
DEFINE_GFX_RTV(GBufferPosDepthTexture);
DEFINE_GFX_DSV(GBufferDepthTexture);

DECLARE_GFX_RESOURCE(GlobalCommonConstants);

GBufferPass::GBufferPass()
    : RenderPass("GBuffer Pass")
{
}

GBufferPass::~GBufferPass()
{
    m_PipelineState.Destroy();
}

void GBufferPass::Initialize()
{
    InitializeShaders();
    InitializeDepthStencilState();
    InitializePipelineState();
}

void GBufferPass::RegisterInputOutput(GraphicContext& context, ResourceContext& rc)
{
    RHIViewportDesc vp = context.GetViewport();
    rc.CreateTexture2DResource(vp.m_Width, vp.m_Height, RHIFormat::R8G8B8A8Unorm, GFX_RESOURCE(GBufferAlbedoTexture));
    rc.CreateTexture2DResource(vp.m_Width, vp.m_Height, RHIFormat::R32G32B32A32Float, GFX_RESOURCE(GBufferNormalTexture));
    rc.CreateTexture2DResource(vp.m_Width, vp.m_Height, RHIFormat::R32G32B32A32Float, GFX_RESOURCE(GBufferPosDepthTexture));
    rc.CreateDepthStencilResource(vp.m_Width, vp.m_Height, RHIFormat::D24UnormS8Uint, GFX_RESOURCE(GBufferDepthTexture));

    rc.CreateRenderTargetView(GFX_RESOURCE(GBufferAlbedoTexture), GFX_RTV(GBufferAlbedoTexture));
    rc.CreateRenderTargetView(GFX_RESOURCE(GBufferNormalTexture), GFX_RTV(GBufferNormalTexture));
    rc.CreateRenderTargetView(GFX_RESOURCE(GBufferPosDepthTexture), GFX_RTV(GBufferPosDepthTexture));
    rc.CreateDepthStencilView(GFX_RESOURCE(GBufferDepthTexture), GFX_DSV(GBufferDepthTexture));
}

void GBufferPass::Render(GraphicContext& context, ResourceContext& rc)
{
    OPTICK_EVENT("GBuffer Pass - Render");

    EngineConfig& config = EngineCore::GetEngineConfig();
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();

    // TODO: Properly support shader hot reload - each PSO should check their own shaders
    if (m_VertexShader->HasRecompiled() || m_PixelShader->HasRecompiled())
    {
        context.GetCommandQueue()->Flush();
        InitializePipelineState();
        m_VertexShader->SetRecompiled(false);
        m_PixelShader->SetRecompiled(false);
    }

    context.TransitionResource(GFX_RESOURCE(GBufferAlbedoTexture), RHIResourceState::RenderTarget);
    context.TransitionResource(GFX_RESOURCE(GBufferNormalTexture), RHIResourceState::RenderTarget);
    context.TransitionResource(GFX_RESOURCE(GBufferPosDepthTexture), RHIResourceState::RenderTarget);

    context.ClearColor(GFX_RTV(GBufferAlbedoTexture), ethVector4());
    context.ClearColor(GFX_RTV(GBufferNormalTexture), ethVector4());
    context.ClearColor(GFX_RTV(GBufferPosDepthTexture), ethVector4());

    RHIRenderTargetViewHandle rtvs[3] = 
    { 
        GFX_RTV(GBufferAlbedoTexture),
        GFX_RTV(GBufferNormalTexture),
        GFX_RTV(GBufferPosDepthTexture)
    };

    context.SetRenderTargets(3, rtvs, GFX_DSV(GBufferDepthTexture));
    context.SetViewport(gfxDisplay.GetViewport());
    context.SetScissor(gfxDisplay.GetScissorRect());

    context.GetCommandList()->SetPipelineState(m_PipelineState);
    context.GetCommandList()->SetGraphicRootSignature(GraphicCore::GetGraphicCommon().m_DefaultRootSignature);
    context.GetCommandList()->SetPrimitiveTopology(RHIPrimitiveTopology::TriangleList);
    context.GetCommandList()->SetStencilRef(255);
    context.GetCommandList()->SetRootConstantBuffer({ 0, GFX_RESOURCE(GlobalCommonConstants) });

    for (auto&& visual : GraphicCore::GetGraphicRenderer().m_PendingVisualNodes)
    {
        ethXMMatrix modelMat = visual->GetModelMatrix();
        ethMatrix3x3 upper3x3;
        DirectX::XMStoreFloat3x3(&upper3x3, modelMat);

        ethXMMatrix normalMat = DirectX::XMLoadFloat3x3(&upper3x3);
        normalMat = DirectX::XMMatrixInverse(nullptr, normalMat);
        normalMat = DirectX::XMMatrixTranspose(normalMat);

        context.GetCommandList()->SetVertexBuffer(visual->GetVertexBufferView());
        context.GetCommandList()->SetIndexBuffer(visual->GetIndexBufferView());

        context.GetCommandList()->SetRootConstants({ 1, sizeof(ethXMMatrix) / 4, 0, &modelMat });
        context.GetCommandList()->SetRootConstants({ 1, sizeof(ethXMMatrix) / 4, 16, &normalMat });
        context.GetCommandList()->DrawIndexedInstanced({ visual->GetNumIndices(), 1, 0, 0, 0 });
    }

    context.FinalizeAndExecute();
    context.Reset();
}

void GBufferPass::InitializeShaders()
{
    m_VertexShader = std::make_unique<Shader>(L"lighting\\gbuffer.hlsl", L"VS_Main", L"vs_6_0", ShaderType::Vertex);
    m_PixelShader = std::make_unique<Shader>(L"lighting\\gbuffer.hlsl", L"PS_Main", L"ps_6_0", ShaderType::Pixel);

    m_VertexShader->Compile();
    m_PixelShader->Compile();
    m_VertexShader->SetRecompiled(false);
    m_PixelShader->SetRecompiled(false);
}

void GBufferPass::InitializeDepthStencilState()
{
    m_DepthStencilState = GraphicCore::GetGraphicCommon().m_DepthStateReadWrite;
    m_DepthStencilState.m_StencilEnabled = true;
    m_DepthStencilState.m_FrontFace.m_StencilFunc = RHIComparator::Always;
    m_DepthStencilState.m_FrontFace.m_StencilPassOp = RHIDepthStencilOperation::Replace;
    m_DepthStencilState.m_BackFace = m_DepthStencilState.m_FrontFace;
}

void GBufferPass::InitializePipelineState()
{
    m_PipelineState.SetName(L"GBufferPass::PipelineState");

    RHIPipelineState creationPSO;
    creationPSO.SetBlendState(GraphicCore::GetGraphicCommon().m_BlendDisabled);
    creationPSO.SetRasterizerState(GraphicCore::GetGraphicCommon().m_RasterizerDefault);
    creationPSO.SetPrimitiveTopology(RHIPrimitiveTopologyType::Triangle);
    creationPSO.SetVertexShader(m_VertexShader->GetCompiledShader(), m_VertexShader->GetCompiledShaderSize());
    creationPSO.SetPixelShader(m_PixelShader->GetCompiledShader(), m_PixelShader->GetCompiledShaderSize());
    creationPSO.SetInputLayout(GraphicCore::GetGraphicCommon().m_DefaultInputLayout);
    RHIFormat formats[3] = { RHIFormat::R8G8B8A8Unorm, RHIFormat::R32G32B32A32Float, RHIFormat::R32G32B32A32Float };
    creationPSO.SetRenderTargetFormats(3, formats);
    creationPSO.SetDepthTargetFormat(RHIFormat::D24UnormS8Uint);
    creationPSO.SetDepthStencilState(m_DepthStencilState);
    creationPSO.SetSamplingDesc(1, 0);
    creationPSO.SetRootSignature(GraphicCore::GetGraphicCommon().m_DefaultRootSignature);
    creationPSO.Finalize(m_PipelineState);
}

void GBufferPass::InitializeRootSignature()
{
    m_RootSignature.SetName(L"GBufferPass::RootSignature");

    RHIRootSignatureFlags rootSignatureFlags =
        static_cast<RHIRootSignatureFlags>(RHIRootSignatureFlag::AllowIAInputLayout) |
        static_cast<RHIRootSignatureFlags>(RHIRootSignatureFlag::DenyHSRootAccess) |
        static_cast<RHIRootSignatureFlags>(RHIRootSignatureFlag::DenyGSRootAccess) |
        static_cast<RHIRootSignatureFlags>(RHIRootSignatureFlag::DenyDSRootAccess);

    RHIRootSignature tempRS(3, 1);
    tempRS[0]->SetAsConstantBufferView({ 0, 0, RHIShaderVisibility::All });
    tempRS[1]->SetAsConstant({ 1, 0, RHIShaderVisibility::Vertex, 32 });
    tempRS.Finalize(rootSignatureFlags, m_RootSignature);

}

ETH_NAMESPACE_END

