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
DEFINE_GFX_RESOURCE(GBufferSpecularTexture);
DEFINE_GFX_RESOURCE(GBufferNormalTexture);
DEFINE_GFX_RESOURCE(GBufferPositionTexture);
DEFINE_GFX_RESOURCE(GBufferDepthTexture);

DEFINE_GFX_RTV(GBufferAlbedoTexture);
DEFINE_GFX_RTV(GBufferSpecularTexture);
DEFINE_GFX_RTV(GBufferNormalTexture);
DEFINE_GFX_RTV(GBufferPositionTexture);
DEFINE_GFX_DSV(GBufferDepthTexture);

DECLARE_GFX_RESOURCE(GlobalCommonConstants);
//DECLARE_GFX_RESOURCE(MaterialConstants);

DEFINE_GFX_RESOURCE(InstanceParams);
DEFINE_GFX_RESOURCE(MaterialParams);
DEFINE_GFX_CBV(InstanceParams);
DEFINE_GFX_CBV(MaterialParams);

#ifdef ETH_TOOLONLY
DEFINE_GFX_RESOURCE(EntityPickerTexture);
DEFINE_GFX_RTV(EntityPickerTexture);
#endif

#define NUM_GBUFFER_RENDER_TARGETS ETH_ENGINE_OR_TOOL(4, 5)

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
    InitializeRootSignature();
    InitializePipelineState();
}

void GBufferPass::RegisterInputOutput(GraphicContext& context, ResourceContext& rc)
{
    RHIViewportDesc vp = context.GetViewport();
    
    rc.CreateTexture2DResource(vp.m_Width, vp.m_Height, RHIFormat::R8G8B8A8Unorm, GFX_RESOURCE(GBufferAlbedoTexture));
    rc.CreateTexture2DResource(vp.m_Width, vp.m_Height, RHIFormat::R8G8B8A8Unorm, GFX_RESOURCE(GBufferSpecularTexture));
    rc.CreateTexture2DResource(vp.m_Width, vp.m_Height, RHIFormat::R32G32B32A32Float, GFX_RESOURCE(GBufferNormalTexture));
    rc.CreateTexture2DResource(vp.m_Width, vp.m_Height, RHIFormat::R32G32B32A32Float, GFX_RESOURCE(GBufferPositionTexture));
    rc.CreateDepthStencilResource(vp.m_Width, vp.m_Height, RHIFormat::D24UnormS8Uint, GFX_RESOURCE(GBufferDepthTexture));

    rc.CreateRenderTargetView(GFX_RESOURCE(GBufferAlbedoTexture), GFX_RTV(GBufferAlbedoTexture));
    rc.CreateRenderTargetView(GFX_RESOURCE(GBufferSpecularTexture), GFX_RTV(GBufferSpecularTexture));
    rc.CreateRenderTargetView(GFX_RESOURCE(GBufferNormalTexture), GFX_RTV(GBufferNormalTexture));
    rc.CreateRenderTargetView(GFX_RESOURCE(GBufferPositionTexture), GFX_RTV(GBufferPositionTexture));
    rc.CreateDepthStencilView(GFX_RESOURCE(GBufferDepthTexture), GFX_DSV(GBufferDepthTexture));

    rc.CreateBufferResource(sizeof(InstanceParams), GFX_RESOURCE(InstanceParams));
    rc.CreateBufferResource(sizeof(MaterialParams), GFX_RESOURCE(MaterialParams));
    rc.CreateConstantBufferView(sizeof(InstanceParams), GFX_RESOURCE(InstanceParams), GFX_CBV(InstanceParams));
    rc.CreateConstantBufferView(sizeof(MaterialParams), GFX_RESOURCE(MaterialParams), GFX_CBV(MaterialParams));

#ifdef ETH_TOOLMODE
    rc.CreateTexture2DResource(vp.m_Width, vp.m_Height, RHIFormat::R8G8B8A8Unorm, GFX_RESOURCE(EntityPickerTexture));
    rc.CreateRenderTargetView(GFX_RESOURCE(EntityPickerTexture), GFX_RTV(EntityPickerTexture));

#endif
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
    context.TransitionResource(GFX_RESOURCE(GBufferSpecularTexture), RHIResourceState::RenderTarget);
    context.TransitionResource(GFX_RESOURCE(GBufferNormalTexture), RHIResourceState::RenderTarget);
    context.TransitionResource(GFX_RESOURCE(GBufferPositionTexture), RHIResourceState::RenderTarget);

    context.ClearColor(GFX_RTV(GBufferAlbedoTexture));
    context.ClearColor(GFX_RTV(GBufferSpecularTexture));
    context.ClearColor(GFX_RTV(GBufferNormalTexture));
    context.ClearColor(GFX_RTV(GBufferPositionTexture));

#ifdef ETH_TOOLMODE
    context.TransitionResource(GFX_RESOURCE(EntityPickerTexture), RHIResourceState::RenderTarget);
    context.ClearColor(GFX_RTV(EntityPickerTexture), ethVector4());
#endif

    RHIRenderTargetViewHandle rtvs[NUM_GBUFFER_RENDER_TARGETS] =
    { 
        GFX_RTV(GBufferAlbedoTexture),
        GFX_RTV(GBufferSpecularTexture),
        GFX_RTV(GBufferNormalTexture),
        GFX_RTV(GBufferPositionTexture),
#ifdef ETH_TOOLMODE
        GFX_RTV(EntityPickerTexture),
#endif
    };

    context.SetRenderTargets(NUM_GBUFFER_RENDER_TARGETS, rtvs, GFX_DSV(GBufferDepthTexture));
    context.SetViewport(gfxDisplay.GetViewport());
    context.SetScissor(gfxDisplay.GetScissorRect());

    context.GetCommandList()->SetPipelineState(m_PipelineState);
    context.GetCommandList()->SetGraphicRootSignature(m_RootSignature);
    context.GetCommandList()->SetDescriptorHeaps({ 1, &GraphicCore::GetSRVDescriptorHeap() });
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



        InstanceParams param;
        DirectX::XMStoreFloat4x4(&param.m_ModelMatrix, modelMat);
        DirectX::XMStoreFloat4x4(&param.m_NormalMatrix, normalMat);
        ETH_TOOLONLY(param.m_PickerColor = visual->GetPickerColor());
        context.InitializeBufferRegion(GFX_RESOURCE(InstanceParams), &param, sizeof(InstanceParams));
        context.GetCommandList()->SetRootConstantBuffer({ 1, GFX_RESOURCE(InstanceParams) });

        MaterialParams mat;
        mat.m_BaseColor = GraphicCore::GetGraphicRenderer().m_BaseColor;
        mat.m_SpecularColor = GraphicCore::GetGraphicRenderer().m_SpecularColor;
        mat.m_Roughness = GraphicCore::GetGraphicRenderer().m_Roughness;
        mat.m_Metalness = GraphicCore::GetGraphicRenderer().m_Metalness;

        context.InitializeBufferRegion(GFX_RESOURCE(MaterialParams), &mat, sizeof(MaterialParams));
        context.GetCommandList()->SetRootConstantBuffer({ 2, GFX_RESOURCE(MaterialParams) });

        // TODO: Setup bindless textures
        // TODO: If null, bind default texture
        auto texture = visual->GetMaterial()->GetTexture("_AlbedoTexture");

        if (texture != nullptr)
        {
			rc.InitializeTexture2D(*texture);
			context.GetCommandList()->SetRootDescriptorTable({ 3, texture->GetView() });
        }

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

    RHIFormat formats[NUM_GBUFFER_RENDER_TARGETS] = { 
        RHIFormat::R8G8B8A8Unorm,
        RHIFormat::R8G8B8A8Unorm,
        RHIFormat::R32G32B32A32Float,
        RHIFormat::R32G32B32A32Float,
#ifdef ETH_TOOLMODE
        RHIFormat::R8G8B8A8Unorm,
#endif
    };

    creationPSO.SetRenderTargetFormats(NUM_GBUFFER_RENDER_TARGETS, formats);
    creationPSO.SetDepthTargetFormat(RHIFormat::D24UnormS8Uint);
    creationPSO.SetDepthStencilState(m_DepthStencilState);
    creationPSO.SetSamplingDesc(1, 0);
    creationPSO.SetRootSignature(m_RootSignature);
    creationPSO.Finalize(m_PipelineState);
}

void GBufferPass::InitializeRootSignature()
{
    m_RootSignature.SetName(L"GBufferPass::RootSignature");

    RHIRootSignature tempRS(4, 3);
    tempRS.GetSampler(0) = GraphicCore::GetGraphicCommon().m_PointSampler;
    tempRS.GetSampler(1) = GraphicCore::GetGraphicCommon().m_BilinearSampler;
    tempRS.GetSampler(2) = GraphicCore::GetGraphicCommon().m_EnvMapSampler;
    tempRS[0]->SetAsConstantBufferView({ 0, 0, RHIShaderVisibility::All });
    tempRS[1]->SetAsConstantBufferView({ 1, 0, RHIShaderVisibility::All });
    tempRS[2]->SetAsConstantBufferView({ 2, 0, RHIShaderVisibility::All });
    tempRS[3]->SetAsDescriptorRange({ 0, 0, RHIShaderVisibility::Pixel, RHIDescriptorRangeType::SRV, 1 });
    tempRS.Finalize(GraphicCore::GetGraphicCommon().m_DefaultRootSignatureFlags, m_RootSignature);

}

ETH_NAMESPACE_END

