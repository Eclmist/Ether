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

#include "gbufferproducer.h"
#include "graphic/rhi/rhicommandlist.h"
#include "graphic/rhi/rhicommandqueue.h"
#include "graphic/rhi/rhidevice.h"
#include "graphic/rhi/rhipipelinestate.h"
#include "graphic/rhi/rhiresource.h"
#include "graphic/rhi/rhirootsignature.h"
#include "graphic/rhi/rhirootparameter.h"

ETH_NAMESPACE_BEGIN

DEFINE_GFX_PASS(GBufferProducer);

DEFINE_GFX_RESOURCE(GBufferAlbedoTexture);
DEFINE_GFX_RESOURCE(GBufferSpecularTexture);
DEFINE_GFX_RESOURCE(GBufferNormalTexture);
DEFINE_GFX_RESOURCE(GBufferPositionTexture);

DEFINE_GFX_RTV(GBufferAlbedoTexture);
DEFINE_GFX_RTV(GBufferSpecularTexture);
DEFINE_GFX_RTV(GBufferNormalTexture);
DEFINE_GFX_RTV(GBufferPositionTexture);

DECLARE_GFX_RESOURCE(GlobalCommonConstants);
//DECLARE_GFX_RESOURCE(MaterialConstants);

DEFINE_GFX_RESOURCE(InstanceParams);
DEFINE_GFX_RESOURCE(MaterialParams);
DEFINE_GFX_CBV(InstanceParams);
DEFINE_GFX_CBV(MaterialParams);
DECLARE_GFX_DSV(DepthStencilTexture);

#ifdef ETH_TOOLONLY
DEFINE_GFX_RESOURCE(EntityPickerTexture);
DEFINE_GFX_RTV(EntityPickerTexture);
#endif

#define NUM_GBUFFER_RENDER_TARGETS ETH_ENGINE_OR_TOOL(4, 5)

GBufferProducer::GBufferProducer()
    : RenderPass("GBuffer Producer")
{
}

GBufferProducer::~GBufferProducer()
{
    m_PipelineState.Destroy();
}

void GBufferProducer::Initialize()
{
    InitializeShaders();
    InitializeDepthStencilState();
    InitializeRootSignature();
    InitializePipelineState();
}

void GBufferProducer::RegisterInputOutput(GraphicContext& context, ResourceContext& rc)
{
    RhiViewportDesc vp = context.GetViewport();
    
    rc.CreateTexture2DResource(vp.m_Width, vp.m_Height, RhiFormat::R8G8B8A8Unorm, GFX_RESOURCE(GBufferAlbedoTexture));
    rc.CreateTexture2DResource(vp.m_Width, vp.m_Height, RhiFormat::R8G8B8A8Unorm, GFX_RESOURCE(GBufferSpecularTexture));
    rc.CreateTexture2DResource(vp.m_Width, vp.m_Height, RhiFormat::R32G32B32A32Float, GFX_RESOURCE(GBufferNormalTexture));
    rc.CreateTexture2DResource(vp.m_Width, vp.m_Height, RhiFormat::R32G32B32A32Float, GFX_RESOURCE(GBufferPositionTexture));

    rc.CreateRenderTargetView(GFX_RESOURCE(GBufferAlbedoTexture), GFX_RTV(GBufferAlbedoTexture));
    rc.CreateRenderTargetView(GFX_RESOURCE(GBufferSpecularTexture), GFX_RTV(GBufferSpecularTexture));
    rc.CreateRenderTargetView(GFX_RESOURCE(GBufferNormalTexture), GFX_RTV(GBufferNormalTexture));
    rc.CreateRenderTargetView(GFX_RESOURCE(GBufferPositionTexture), GFX_RTV(GBufferPositionTexture));

    rc.CreateConstantBuffer(sizeof(InstanceParams), GFX_RESOURCE(InstanceParams));
    rc.CreateConstantBuffer(sizeof(MaterialParams), GFX_RESOURCE(MaterialParams));
    rc.CreateConstantBufferView(sizeof(InstanceParams), GFX_RESOURCE(InstanceParams), GFX_CBV(InstanceParams));
    rc.CreateConstantBufferView(sizeof(MaterialParams), GFX_RESOURCE(MaterialParams), GFX_CBV(MaterialParams));

#ifdef ETH_TOOLMODE
    rc.CreateTexture2DResource(vp.m_Width, vp.m_Height, RhiFormat::R8G8B8A8Unorm, GFX_RESOURCE(EntityPickerTexture));
    rc.CreateRenderTargetView(GFX_RESOURCE(EntityPickerTexture), GFX_RTV(EntityPickerTexture));

#endif
}

void GBufferProducer::Render(GraphicContext& context, ResourceContext& rc)
{
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

    context.TransitionResource(GFX_RESOURCE(GBufferAlbedoTexture), RhiResourceState::RenderTarget);
    context.TransitionResource(GFX_RESOURCE(GBufferSpecularTexture), RhiResourceState::RenderTarget);
    context.TransitionResource(GFX_RESOURCE(GBufferNormalTexture), RhiResourceState::RenderTarget);
    context.TransitionResource(GFX_RESOURCE(GBufferPositionTexture), RhiResourceState::RenderTarget);

    context.ClearColor(GFX_RTV(GBufferAlbedoTexture));
    context.ClearColor(GFX_RTV(GBufferSpecularTexture));
    context.ClearColor(GFX_RTV(GBufferNormalTexture));
    context.ClearColor(GFX_RTV(GBufferPositionTexture));

#ifdef ETH_TOOLMODE
    context.TransitionResource(GFX_RESOURCE(EntityPickerTexture), RhiResourceState::RenderTarget);
    context.ClearColor(GFX_RTV(EntityPickerTexture), ethVector4());
#endif

    RhiRenderTargetViewHandle rtvs[NUM_GBUFFER_RENDER_TARGETS] =
    { 
        GFX_RTV(GBufferAlbedoTexture),
        GFX_RTV(GBufferSpecularTexture),
        GFX_RTV(GBufferNormalTexture),
        GFX_RTV(GBufferPositionTexture),
#ifdef ETH_TOOLMODE
        GFX_RTV(EntityPickerTexture),
#endif
    };

    context.SetRenderTargets(NUM_GBUFFER_RENDER_TARGETS, rtvs, GFX_DSV(DepthStencilTexture));
    context.SetViewport(gfxDisplay.GetViewport());
    context.SetScissor(gfxDisplay.GetScissorRect());

    context.GetCommandList()->SetPipelineState(m_PipelineState);
    context.GetCommandList()->SetGraphicRootSignature(m_RootSignature);
    context.GetCommandList()->SetDescriptorHeaps({ 1, &GraphicCore::GetSRVDescriptorHeap() });
    context.GetCommandList()->SetPrimitiveTopology(RhiPrimitiveTopology::TriangleList);
    context.GetCommandList()->SetStencilRef(255);
    context.GetCommandList()->SetRootConstantBuffer({ 0, GFX_RESOURCE(GlobalCommonConstants) });

    void *mappedInstanceParams, *mappedMaterialParams;
    GFX_RESOURCE(InstanceParams)->Map(&mappedInstanceParams);
    GFX_RESOURCE(MaterialParams)->Map(&mappedMaterialParams);

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

        InstanceParams params;
        DirectX::XMStoreFloat4x4(&params.m_ModelMatrix, modelMat);
        DirectX::XMStoreFloat4x4(&params.m_NormalMatrix, normalMat);
        ETH_TOOLONLY(params.m_PickerColor = visual->GetPickerColor());
        memcpy(mappedInstanceParams, &params, sizeof(params));
        context.GetCommandList()->SetRootConstantBuffer({ 1, GFX_RESOURCE(InstanceParams) });

        MaterialParams mat;
        mat.m_BaseColor = GraphicCore::GetGraphicRenderer().m_BaseColor;
        mat.m_SpecularColor = GraphicCore::GetGraphicRenderer().m_SpecularColor;
        mat.m_Roughness = GraphicCore::GetGraphicRenderer().m_Roughness;
        mat.m_Metalness = GraphicCore::GetGraphicRenderer().m_Metalness;
        memcpy(mappedMaterialParams, &mat, sizeof(mat));
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

    GFX_RESOURCE(InstanceParams)->Unmap();
    GFX_RESOURCE(MaterialParams)->Unmap();

    context.FinalizeAndExecute();
    context.Reset();
}

void GBufferProducer::InitializeShaders()
{
    m_VertexShader = std::make_unique<Shader>(L"lighting\\gbuffer.hlsl", L"VS_Main", L"vs_6_0", ShaderType::Vertex);
    m_PixelShader = std::make_unique<Shader>(L"lighting\\gbuffer.hlsl", L"PS_Main", L"ps_6_0", ShaderType::Pixel);

    m_VertexShader->Compile();
    m_PixelShader->Compile();
    m_VertexShader->SetRecompiled(false);
    m_PixelShader->SetRecompiled(false);
}

void GBufferProducer::InitializeDepthStencilState()
{
    m_DepthStencilState = GraphicCore::GetGraphicCommon().m_DepthStateReadWrite;
    m_DepthStencilState.m_StencilEnabled = true;
    m_DepthStencilState.m_FrontFace.m_StencilFunc = RhiComparator::Always;
    m_DepthStencilState.m_FrontFace.m_StencilPassOp = RhiDepthStencilOperation::Replace;
    m_DepthStencilState.m_BackFace = m_DepthStencilState.m_FrontFace;
}

void GBufferProducer::InitializePipelineState()
{
    m_PipelineState.SetName(L"GBufferPass::PipelineState");

    RhiPipelineState creationPSO;
    creationPSO.SetBlendState(GraphicCore::GetGraphicCommon().m_BlendDisabled);
    creationPSO.SetRasterizerState(GraphicCore::GetGraphicCommon().m_RasterizerDefault);
    creationPSO.SetPrimitiveTopology(RhiPrimitiveTopologyType::Triangle);
    creationPSO.SetVertexShader(m_VertexShader->GetCompiledShader(), m_VertexShader->GetCompiledShaderSize());
    creationPSO.SetPixelShader(m_PixelShader->GetCompiledShader(), m_PixelShader->GetCompiledShaderSize());
    creationPSO.SetInputLayout(GraphicCore::GetGraphicCommon().m_DefaultInputLayout);

    RhiFormat formats[NUM_GBUFFER_RENDER_TARGETS] = { 
        RhiFormat::R8G8B8A8Unorm,
        RhiFormat::R8G8B8A8Unorm,
        RhiFormat::R32G32B32A32Float,
        RhiFormat::R32G32B32A32Float,
#ifdef ETH_TOOLMODE
        RhiFormat::R8G8B8A8Unorm,
#endif
    };

    creationPSO.SetRenderTargetFormats(NUM_GBUFFER_RENDER_TARGETS, formats);
    creationPSO.SetDepthTargetFormat(RhiFormat::D24UnormS8Uint);
    creationPSO.SetDepthStencilState(m_DepthStencilState);
    creationPSO.SetSamplingDesc(1, 0);
    creationPSO.SetRootSignature(m_RootSignature);
    creationPSO.Finalize(m_PipelineState);
}

void GBufferProducer::InitializeRootSignature()
{
    m_RootSignature.SetName(L"GBufferPass::RootSignature");

    RhiRootSignature tempRS(4, 3);
    tempRS.GetSampler(0) = GraphicCore::GetGraphicCommon().m_PointSampler;
    tempRS.GetSampler(1) = GraphicCore::GetGraphicCommon().m_BilinearSampler;
    tempRS.GetSampler(2) = GraphicCore::GetGraphicCommon().m_EnvMapSampler;
    tempRS[0]->SetAsConstantBufferView({ 0, 0, RhiShaderVisibility::All });
    tempRS[1]->SetAsConstantBufferView({ 1, 0, RhiShaderVisibility::All });
    tempRS[2]->SetAsConstantBufferView({ 2, 0, RhiShaderVisibility::All });
    tempRS[3]->SetAsDescriptorRange({ 0, 0, RhiShaderVisibility::Pixel, RhiDescriptorRangeType::SRV, 1 });
    tempRS.Finalize(GraphicCore::GetGraphicCommon().m_DefaultRootSignatureFlags, m_RootSignature);

}

ETH_NAMESPACE_END

