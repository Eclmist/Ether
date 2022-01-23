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

#include "deferredlightingpass.h"
#include "graphic/rhi/rhicommandlist.h"
#include "graphic/rhi/rhicommandqueue.h"
#include "graphic/rhi/rhipipelinestate.h"
#include "graphic/rhi/rhirootparameter.h"
#include "graphic/rhi/rhirootsignature.h"

ETH_NAMESPACE_BEGIN

DEFINE_GFX_PASS(DeferredLightingPass);

DECLARE_GFX_RESOURCE(GBufferAlbedoTexture);
DECLARE_GFX_RESOURCE(GBufferNormalTexture);
DECLARE_GFX_RESOURCE(GBufferPosDepthTexture);

DEFINE_GFX_SRV(GBufferAlbedoTexture);
DEFINE_GFX_SRV(GBufferNormalTexture);
DEFINE_GFX_SRV(GBufferPosDepthTexture);

DECLARE_GFX_DSV(GBufferDepthTexture);

DeferredLightingPass::DeferredLightingPass()
    : RenderPass("Deferred Lighting Pass")
{
}

void DeferredLightingPass::Initialize()
{
    InitializeShaders();
    InitializeDepthStencilState();
    InitializeRootSignature();
    InitializePipelineState();
}

void DeferredLightingPass::RegisterInputOutput(GraphicContext& context, ResourceContext& rc)
{
    rc.CreateShaderResourceView(GFX_RESOURCE(GBufferAlbedoTexture), GFX_SRV(GBufferAlbedoTexture));
    rc.CreateShaderResourceView(GFX_RESOURCE(GBufferNormalTexture), GFX_SRV(GBufferNormalTexture));
    rc.CreateShaderResourceView(GFX_RESOURCE(GBufferPosDepthTexture), GFX_SRV(GBufferPosDepthTexture));
}

void DeferredLightingPass::Render(GraphicContext& context, ResourceContext& rc)
{
    OPTICK_EVENT("Deferred Lighting Pass - Render");

    // TODO: Properly support shader hot reload - each PSO should check their own shaders
    if (m_VertexShader->HasRecompiled() || m_PixelShader->HasRecompiled())
    {
        context.GetCommandQueue()->Flush();
        InitializePipelineState();
        m_VertexShader->SetRecompiled(false);
        m_PixelShader->SetRecompiled(false);
    }

    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    context.SetRenderTarget(gfxDisplay.GetCurrentBackBufferRTV(), GFX_DSV(GBufferDepthTexture));
    context.SetViewport(gfxDisplay.GetViewport());
    context.SetScissor(gfxDisplay.GetScissorRect());

    context.GetCommandList()->SetPipelineState(m_PipelineState);
    context.GetCommandList()->SetGraphicRootSignature(m_RootSignature);
    context.GetCommandList()->SetPrimitiveTopology(RHIPrimitiveTopology::TriangleStrip);
    context.GetCommandList()->SetStencilRef(255);

    ethXMVector globalTime = DirectX::XMVectorSet(GetTimeSinceStart() / 20, GetTimeSinceStart(), GetTimeSinceStart() * 2, GetTimeSinceStart() * 3);
    context.GetCommandList()->SetDescriptorHeaps({ 1, &GraphicCore::GetSRVDescriptorHeap() });
    context.GetCommandList()->SetRootConstants({ 1, 4, 0, &globalTime });
    context.GetCommandList()->SetRootConstants({ 2, 4, 0, &context.GetEyeDirection() });

    // TODO: This technically binds 3 SRVs - as specified in the descriptor range in InitializeRootSignature()
    // There doesn't seem to be a way to explicitly bind textures one at a time unless each one is a table,
    // which is quite eww. Therefore all the textures just have to been sequential in the SRV Heap implicitly.
    // This will break depending on how the descriptor allcator is written later, how do we fix this?
    context.GetCommandList()->SetRootDescriptorTable({ 0, GFX_SRV(GBufferAlbedoTexture) });
    context.GetCommandList()->DrawInstanced({ 4, 1, 0, 0 });

    context.FinalizeAndExecute();
    context.Reset();
}

void DeferredLightingPass::InitializeShaders()
{
    m_VertexShader = std::make_unique<Shader>(L"lighting\\deferredlights.hlsl", L"VS_Main", L"vs_6_0", ShaderType::Vertex);
    m_PixelShader = std::make_unique<Shader>(L"lighting\\deferredlights.hlsl", L"PS_Main", L"ps_6_0", ShaderType::Pixel);

    m_VertexShader->Compile();
    m_PixelShader->Compile();
    m_VertexShader->SetRecompiled(false);
    m_PixelShader->SetRecompiled(false);
}

void DeferredLightingPass::InitializeDepthStencilState()
{
    m_DepthStencilState = GraphicCore::GetGraphicCommon().m_DepthStateReadWrite;
    m_DepthStencilState.m_StencilEnabled = true;
    m_DepthStencilState.m_FrontFace.m_StencilFunc = RHIComparator::Equal;
    m_DepthStencilState.m_FrontFace.m_StencilPassOp = RHIDepthStencilOperation::Keep;
    m_DepthStencilState.m_FrontFace.m_StencilFailOp = RHIDepthStencilOperation::Keep;
    m_DepthStencilState.m_BackFace = m_DepthStencilState.m_FrontFace;
    m_DepthStencilState.m_DepthComparator = RHIComparator::Always;
}

void DeferredLightingPass::InitializePipelineState()
{
    m_PipelineState.SetName(L"DeferredLightingPass::PipelineState");

    RHIPipelineState creationPSO;
    creationPSO.SetBlendState(GraphicCore::GetGraphicCommon().m_BlendDisabled);
    creationPSO.SetRasterizerState(GraphicCore::GetGraphicCommon().m_RasterizerDefault);
    creationPSO.SetPrimitiveTopology(RHIPrimitiveTopologyType::Triangle);
    creationPSO.SetVertexShader(m_VertexShader->GetCompiledShader(), m_VertexShader->GetCompiledShaderSize());
    creationPSO.SetPixelShader(m_PixelShader->GetCompiledShader(), m_PixelShader->GetCompiledShaderSize());
    creationPSO.SetInputLayout(GraphicCore::GetGraphicCommon().m_DefaultInputLayout);
    creationPSO.SetRenderTargetFormat(RHIFormat::R8G8B8A8Unorm);
    creationPSO.SetDepthTargetFormat(RHIFormat::D24UnormS8Uint);
    creationPSO.SetDepthStencilState(m_DepthStencilState);
    creationPSO.SetSamplingDesc(1, 0);
    creationPSO.SetRootSignature(m_RootSignature);
    creationPSO.Finalize(m_PipelineState);
}

void DeferredLightingPass::InitializeRootSignature()
{
    m_RootSignature.SetName(L"GBufferPass::RootSignature");

    RHIRootSignatureFlags rootSignatureFlags =
        static_cast<RHIRootSignatureFlags>(RHIRootSignatureFlag::AllowIAInputLayout) |
        static_cast<RHIRootSignatureFlags>(RHIRootSignatureFlag::DenyHSRootAccess) |
        static_cast<RHIRootSignatureFlags>(RHIRootSignatureFlag::DenyGSRootAccess) |
        static_cast<RHIRootSignatureFlags>(RHIRootSignatureFlag::DenyDSRootAccess);

    RHIRootSignature tempRS(3, 1);
    RHIDescriptorRangeDesc rangeDesc = {};
    rangeDesc.m_NumDescriptors = 3;
    rangeDesc.m_ShaderRegister = 0;
    rangeDesc.m_ShaderVisibility = RHIShaderVisibility::Pixel;
    rangeDesc.m_Type = RHIDescriptorRangeType::SRV;

    RHISamplerParameterDesc& sampler = tempRS.GetSampler(0);
    sampler.m_Filter = RHIFilter::MinMagMipPoint;
    sampler.m_AddressU = RHITextureAddressMode::Clamp;
    sampler.m_AddressV = RHITextureAddressMode::Clamp;
    sampler.m_AddressW = RHITextureAddressMode::Clamp;
    sampler.m_MipLODBias = 0;
    sampler.m_MaxAnisotropy = 0;
    sampler.m_ComparisonFunc = RHIComparator::Never;
    sampler.m_MinLOD = 0;
    sampler.m_MinLOD = std::numeric_limits<float_t>().max();
    sampler.m_ShaderRegister = 0;
    sampler.m_RegisterSpace = 0;
    sampler.m_ShaderVisibility = RHIShaderVisibility::Pixel;

    tempRS[0]->SetAsDescriptorRange(rangeDesc);
    tempRS[1]->SetAsConstant({ 0, 0, RHIShaderVisibility::All, 4 });
    tempRS[2]->SetAsConstant({ 1, 0, RHIShaderVisibility::All, 4 });

    tempRS.Finalize(rootSignatureFlags, m_RootSignature);
}

ETH_NAMESPACE_END

