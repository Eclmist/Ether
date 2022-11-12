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

#include "deferredlightingproducer.h"
#include "graphic/rhi/rhicommandlist.h"
#include "graphic/rhi/rhicommandqueue.h"
#include "graphic/rhi/rhipipelinestate.h"
#include "graphic/rhi/rhirootparameter.h"
#include "graphic/rhi/rhirootsignature.h"

ETH_NAMESPACE_BEGIN

DEFINE_GFX_PASS(DeferredLightingProducer);

DECLARE_GFX_RESOURCE(GlobalCommonConstants);
DECLARE_GFX_RESOURCE(GBufferAlbedoTexture);
DECLARE_GFX_RESOURCE(GBufferSpecularTexture);
DECLARE_GFX_RESOURCE(GBufferNormalTexture);
DECLARE_GFX_RESOURCE(GBufferPositionTexture);
DECLARE_GFX_DSV(DepthStencilTexture);

DEFINE_GFX_SRV(GBufferAlbedoTexture);
DEFINE_GFX_SRV(GBufferSpecularTexture);
DEFINE_GFX_SRV(GBufferNormalTexture);
DEFINE_GFX_SRV(GBufferPositionTexture);

DEFINE_GFX_RESOURCE(GBufferIndices);
DEFINE_GFX_CBV(GBufferIndices);

DeferredLightingProducer::DeferredLightingProducer()
    : RenderPass("Deferred Lighting Producer")
{
}

void DeferredLightingProducer::Initialize()
{
    InitializeShaders();
    InitializeDepthStencilState();
    InitializePipelineState();
}

void DeferredLightingProducer::RegisterInputOutput(GraphicContext& context, ResourceContext& rc)
{
    rc.CreateConstantBuffer(sizeof(GBufferIndices), GFX_RESOURCE(GBufferIndices));
    rc.CreateConstantBufferView(sizeof(GBufferIndices), GFX_RESOURCE(GBufferIndices), GFX_CBV(GBufferIndices));

    rc.CreateShaderResourceView(GFX_RESOURCE(GBufferAlbedoTexture), GFX_SRV(GBufferAlbedoTexture));
    rc.CreateShaderResourceView(GFX_RESOURCE(GBufferSpecularTexture), GFX_SRV(GBufferSpecularTexture));
    rc.CreateShaderResourceView(GFX_RESOURCE(GBufferNormalTexture), GFX_SRV(GBufferNormalTexture));
    rc.CreateShaderResourceView(GFX_RESOURCE(GBufferPositionTexture), GFX_SRV(GBufferPositionTexture));
}

void DeferredLightingProducer::Render(GraphicContext& context, ResourceContext& rc)
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
    context.SetRenderTarget(gfxDisplay.GetCurrentBackBufferRtv(), GFX_DSV(DepthStencilTexture));
    context.SetViewport(gfxDisplay.GetViewport());
    context.SetScissor(gfxDisplay.GetScissorRect());

    context.GetCommandList()->SetPipelineState(m_PipelineState);
    context.GetCommandList()->SetGraphicRootSignature(GraphicCore::GetGraphicCommon().m_BindlessRootSignature);
    context.GetCommandList()->SetPrimitiveTopology(RhiPrimitiveTopology::TriangleStrip);
    context.GetCommandList()->SetStencilRef(255);
    context.GetCommandList()->SetDescriptorHeaps({ 1, &GraphicCore::GetGpuDescriptorAllocator().GetDescriptorHeap() });
    context.GetCommandList()->SetRootConstantBuffer({ 0, GFX_RESOURCE(GlobalCommonConstants) });

    void* mappedTextureIndices;
    GFX_RESOURCE(GBufferIndices)->Map(&mappedTextureIndices);

    GBufferIndices gbufferIndices = {};
    gbufferIndices.m_AlbedoTextureIndex = GraphicCore::GetBindlessResourceManager().GetViewIndex(GFX_SRV(GBufferAlbedoTexture));
    gbufferIndices.m_SpecularTextureIndex = GraphicCore::GetBindlessResourceManager().GetViewIndex(GFX_SRV(GBufferSpecularTexture));
    gbufferIndices.m_NormalTextureIndex = GraphicCore::GetBindlessResourceManager().GetViewIndex(GFX_SRV(GBufferNormalTexture));
    gbufferIndices.m_PositionTextureIndex = GraphicCore::GetBindlessResourceManager().GetViewIndex(GFX_SRV(GBufferPositionTexture));

    auto texture = GraphicCore::GetGraphicRenderer().m_EnvironmentHDRI;
    if (texture != nullptr)
    {
        rc.InitializeTexture2D(*texture);
        gbufferIndices.m_EnvironmentHdriTextureIndex = GraphicCore::GetBindlessResourceManager().GetViewIndex(texture->GetView());
    }

    memcpy(mappedTextureIndices, &gbufferIndices, sizeof(GBufferIndices));
    context.GetCommandList()->SetRootConstantBuffer({ 1, GFX_RESOURCE(GBufferIndices) });
    context.GetCommandList()->DrawInstanced({ 4, 1, 0, 0 });
    context.FinalizeAndExecute();
    context.Reset();
}

void DeferredLightingProducer::InitializeShaders()
{
    m_VertexShader = std::make_unique<Shader>(L"lighting\\deferredlights.hlsl", L"VS_Main", L"vs_6_6", ShaderType::Vertex);
    m_PixelShader = std::make_unique<Shader>(L"lighting\\deferredlights.hlsl", L"PS_Main", L"ps_6_6", ShaderType::Pixel);

    m_VertexShader->Compile();
    m_PixelShader->Compile();
    m_VertexShader->SetRecompiled(false);
    m_PixelShader->SetRecompiled(false);
}

void DeferredLightingProducer::InitializeDepthStencilState()
{
    m_DepthStencilState = GraphicCore::GetGraphicCommon().m_DepthStateReadOnly;
    m_DepthStencilState.m_StencilEnabled = true;
    m_DepthStencilState.m_FrontFace.m_StencilFunc = RhiComparator::Equal;
    m_DepthStencilState.m_FrontFace.m_StencilPassOp = RhiDepthStencilOperation::Keep;
    m_DepthStencilState.m_FrontFace.m_StencilFailOp = RhiDepthStencilOperation::Keep;
    m_DepthStencilState.m_BackFace = m_DepthStencilState.m_FrontFace;
    m_DepthStencilState.m_DepthComparator = RhiComparator::Always;
}

void DeferredLightingProducer::InitializePipelineState()
{
    m_PipelineState.SetName(L"DeferredLightingPass::PipelineState");

    RhiPipelineState creationPSO;
    creationPSO.SetBlendState(GraphicCore::GetGraphicCommon().m_BlendDisabled);
    creationPSO.SetRasterizerState(GraphicCore::GetGraphicCommon().m_RasterizerDefault);
    creationPSO.SetPrimitiveTopology(RhiPrimitiveTopologyType::Triangle);
    creationPSO.SetVertexShader(m_VertexShader->GetCompiledShader(), m_VertexShader->GetCompiledShaderSize());
    creationPSO.SetPixelShader(m_PixelShader->GetCompiledShader(), m_PixelShader->GetCompiledShaderSize());
    creationPSO.SetInputLayout(GraphicCore::GetGraphicCommon().m_DefaultInputLayout);
    creationPSO.SetRenderTargetFormat(BackBufferFormat);
    creationPSO.SetDepthTargetFormat(RhiFormat::D24UnormS8Uint);
    creationPSO.SetDepthStencilState(m_DepthStencilState);
    creationPSO.SetSamplingDesc(1, 0);
    creationPSO.SetRootSignature(GraphicCore::GetGraphicCommon().m_BindlessRootSignature);
    creationPSO.Finalize(m_PipelineState);
}

ETH_NAMESPACE_END

