///*
//    This file is part of Ether, an open-source DirectX 12 renderer.
//
//    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.
//
//    Ether is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program. If not, see <http://www.gnu.org/licenses/>.
//*/
//
//#include "bloompass.h"
//#include "graphic/rhi/rhicommandlist.h"
//#include "graphic/rhi/rhicommandqueue.h"
//#include "graphic/rhi/rhipipelinestate.h"
//#include "graphic/rhi/rhirootparameter.h"
//#include "graphic/rhi/rhirootsignature.h"
//
//ETH_NAMESPACE_BEGIN
//
//DEFINE_GFX_PASS(BloomPass);
//
//DECLARE_GFX_RESOURCE(GlobalCommonConstants);
//
//DEFINE_GFX_RESOURCE(HighPassFilter);
//DEFINE_GFX_RESOURCE(BlurTex1);
//DEFINE_GFX_RESOURCE(BlurTex2);
//DEFINE_GFX_RESOURCE(BlurTex3);
//DEFINE_GFX_RESOURCE(BlurTex4);
//DEFINE_GFX_RESOURCE(BlurTex5);
//DEFINE_GFX_RESOURCE(BlurTex6);
//
//DEFINE_GFX_RTV(HighPassFilter);
//DEFINE_GFX_RTV(BlurTex1);
//DEFINE_GFX_RTV(BlurTex2);
//DEFINE_GFX_RTV(BlurTex3);
//DEFINE_GFX_RTV(BlurTex4);
//DEFINE_GFX_RTV(BlurTex5);
//DEFINE_GFX_RTV(BlurTex6);
//
//DEFINE_GFX_SRV(HighPassFilter);
//DEFINE_GFX_SRV(BlurTex1);
//DEFINE_GFX_SRV(BlurTex2);
//DEFINE_GFX_SRV(BlurTex3);
//DEFINE_GFX_SRV(BlurTex4);
//DEFINE_GFX_SRV(BlurTex5);
//DEFINE_GFX_SRV(BlurTex6);
//
//BloomPass::BloomPass()
//    : RenderPass("Bloom Pass")
//{
//}
//
//void BloomPass::Initialize()
//{
//    InitializeShaders();
//    InitializeRootSignature();
//    InitializePipelineState();
//}
//
//void BloomPass::RegisterInputOutput(GraphicContext& context, ResourceContext& rc)
//{
//    RhiViewportDesc vp = context.GetViewport();
//
//    rc.CreateTexture2DResource(vp.m_Width, vp.m_Height, BackBufferFormat, GFX_RESOURCE(HighPassFilter));
//    rc.CreateTexture2DResource(vp.m_Width, vp.m_Height, BackBufferFormat, GFX_RESOURCE(BlurTex1));
//    rc.CreateTexture2DResource(vp.m_Width / 2, vp.m_Height / 2, BackBufferFormat, GFX_RESOURCE(BlurTex2));
//    rc.CreateTexture2DResource(vp.m_Width / 4, vp.m_Height / 4, BackBufferFormat, GFX_RESOURCE(BlurTex3));
//    rc.CreateTexture2DResource(vp.m_Width / 8, vp.m_Height / 8, BackBufferFormat, GFX_RESOURCE(BlurTex4));
//    rc.CreateTexture2DResource(vp.m_Width / 16, vp.m_Height / 16, BackBufferFormat, GFX_RESOURCE(BlurTex5));
//    rc.CreateTexture2DResource(vp.m_Width / 32, vp.m_Height / 32, BackBufferFormat, GFX_RESOURCE(BlurTex6));
//
//    rc.CreateRenderTargetView(GFX_RESOURCE(HighPassFilter), GFX_RTV(HighPassFilter));
//    rc.CreateRenderTargetView(GFX_RESOURCE(BlurTex1), GFX_RTV(BlurTex1));
//    rc.CreateRenderTargetView(GFX_RESOURCE(BlurTex2), GFX_RTV(BlurTex2));
//    rc.CreateRenderTargetView(GFX_RESOURCE(BlurTex3), GFX_RTV(BlurTex3));
//    rc.CreateRenderTargetView(GFX_RESOURCE(BlurTex4), GFX_RTV(BlurTex4));
//    rc.CreateRenderTargetView(GFX_RESOURCE(BlurTex5), GFX_RTV(BlurTex5));
//    rc.CreateRenderTargetView(GFX_RESOURCE(BlurTex6), GFX_RTV(BlurTex6));
//
//    rc.CreateShaderResourceView(GFX_RESOURCE(HighPassFilter), GFX_SRV(HighPassFilter));
//    rc.CreateShaderResourceView(GFX_RESOURCE(BlurTex1), GFX_SRV(BlurTex1));
//    rc.CreateShaderResourceView(GFX_RESOURCE(BlurTex2), GFX_SRV(BlurTex2));
//    rc.CreateShaderResourceView(GFX_RESOURCE(BlurTex3), GFX_SRV(BlurTex3));
//    rc.CreateShaderResourceView(GFX_RESOURCE(BlurTex4), GFX_SRV(BlurTex4));
//    rc.CreateShaderResourceView(GFX_RESOURCE(BlurTex5), GFX_SRV(BlurTex5));
//    rc.CreateShaderResourceView(GFX_RESOURCE(BlurTex6), GFX_SRV(BlurTex6));
//}
//
//void BloomPass::Render(GraphicContext& context, ResourceContext& rc)
//{
//    // TODO: Properly support shader hot reload - each PSO should check their own shaders
//    if (m_GaussianVS->HasRecompiled() || m_BloomCompositePS->HasRecompiled())
//    {
//        context.GetCommandQueue()->Flush();
//        InitializePipelineState();
//        m_GaussianVS->SetRecompiled(false);
//        m_GaussianHorizontalPS->SetRecompiled(false);
//        m_BloomHighpassPS->SetRecompiled(false);
//        m_BloomCompositePS->SetRecompiled(false);
//    }
//
//    //context.ClearColor(GFX_RTV(HighPassFilter));
//    //context.ClearColor(GFX_RTV(BlurTex1));
//    //context.ClearColor(GFX_RTV(BlurTex2));
//    //context.ClearColor(GFX_RTV(BlurTex3));
//    //context.ClearColor(GFX_RTV(BlurTex4));
//    //context.ClearColor(GFX_RTV(BlurTex5));
//    //context.ClearColor(GFX_RTV(BlurTex6));
//
//    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
//
//    RhiViewportDesc viewport = gfxDisplay.GetViewport();
//    //context.TransitionResource(GFX_RESOURCE(DeferredLightingOutput), RhiResourceState::GenericRead);
//    context.TransitionResource(GFX_RESOURCE(HighPassFilter), RhiResourceState::RenderTarget);
//    context.SetViewport(viewport);
//    context.SetScissor(gfxDisplay.GetScissorRect());
//    context.GetCommandList()->SetGraphicRootSignature(m_RootSignature);
//    context.GetCommandList()->SetPrimitiveTopology(RhiPrimitiveTopology::TriangleStrip);
//    context.GetCommandList()->SetStencilRef(255);
//    context.GetCommandList()->SetDescriptorHeaps({ 1, &GraphicCore::GetDescriptorManager().GetDescriptorHeap() });
//    context.GetCommandList()->SetRootConstantBuffer({ 0, GFX_RESOURCE(GlobalCommonConstants) });
//    context.SetRenderTarget(GFX_RTV(HighPassFilter));
//    context.GetCommandList()->SetPipelineState(m_PipelineStateBloomHighpass);
//    //context.GetCommandList()->SetRootDescriptorTable({ 1, gfxDisplay.GetCurrentBackBufferSrv() });
//    context.GetCommandList()->DrawInstanced({ 4, 1, 0, 0 });
//
//    context.SetViewport(viewport);
//    context.SetScissor(gfxDisplay.GetScissorRect());
//    //context.TransitionResource(GFX_RESOURCE(HighPassFilter), RhiResourceState::GenericRead);
//    context.TransitionResource(GFX_RESOURCE(BlurTex1), RhiResourceState::RenderTarget);
//    GaussianHorizontalPass(context, GFX_SRV(HighPassFilter), GFX_RTV(BlurTex1));
//    //context.SetViewport(viewport);
//    //context.SetScissor(gfxDisplay.GetScissorRect());
//    //GaussianVerticalPass(context, GFX_SRV(BlurTex1), GFX_RTV(BlurTex1));
//
//    viewport.m_Width /= 2;
//    viewport.m_Height /= 2;
//    context.SetViewport(viewport);
//    context.SetScissor(gfxDisplay.GetScissorRect());
//    //context.TransitionResource(GFX_RESOURCE(BlurTex1), RhiResourceState::GenericRead);
//    context.TransitionResource(GFX_RESOURCE(BlurTex2), RhiResourceState::RenderTarget);
//
//    GaussianHorizontalPass(context, GFX_SRV(BlurTex1), GFX_RTV(BlurTex2));
//    //context.SetViewport(viewport);
//    //context.SetScissor(gfxDisplay.GetScissorRect());
//    //GaussianVerticalPass(context, GFX_SRV(BlurTex2), GFX_RTV(BlurTex2));
//
//    viewport.m_Width /= 2;
//    viewport.m_Height /= 2;
//    context.SetViewport(viewport);
//    context.SetScissor(gfxDisplay.GetScissorRect());
//    //context.TransitionResource(GFX_RESOURCE(BlurTex2), RhiResourceState::GenericRead);
//    context.TransitionResource(GFX_RESOURCE(BlurTex3), RhiResourceState::RenderTarget);
//
//    GaussianHorizontalPass(context, GFX_SRV(BlurTex2), GFX_RTV(BlurTex3));
//    //context.SetViewport(viewport);
//    //context.SetScissor(gfxDisplay.GetScissorRect());
//    //GaussianVerticalPass(context, GFX_SRV(BlurTex3), GFX_RTV(BlurTex3));
//
//    viewport.m_Width /= 2;
//    viewport.m_Height /= 2;
//    context.SetViewport(viewport);
//    context.SetScissor(gfxDisplay.GetScissorRect());
//    //context.TransitionResource(GFX_RESOURCE(BlurTex3), RhiResourceState::GenericRead);
//    context.TransitionResource(GFX_RESOURCE(BlurTex4), RhiResourceState::RenderTarget);
//
//    GaussianHorizontalPass(context, GFX_SRV(BlurTex3), GFX_RTV(BlurTex4));
//    //context.SetViewport(viewport);
//    //context.SetScissor(gfxDisplay.GetScissorRect());
//    //GaussianVerticalPass(context, GFX_SRV(BlurTex4), GFX_RTV(BlurTex4));
//
//    viewport.m_Width /= 2;
//    viewport.m_Height /= 2;
//    context.SetViewport(viewport);
//    context.SetScissor(gfxDisplay.GetScissorRect());
//    //context.TransitionResource(GFX_RESOURCE(BlurTex4), RhiResourceState::GenericRead);
//    context.TransitionResource(GFX_RESOURCE(BlurTex5), RhiResourceState::RenderTarget);
//
//    GaussianHorizontalPass(context, GFX_SRV(BlurTex4), GFX_RTV(BlurTex5));
//    //context.SetViewport(viewport);
//    //context.SetScissor(gfxDisplay.GetScissorRect());
//    //GaussianVerticalPass(context, GFX_SRV(BlurTex5), GFX_RTV(BlurTex5));
//
//    viewport.m_Width /= 2;
//    viewport.m_Height /= 2;
//    context.SetViewport(viewport);
//    context.SetScissor(gfxDisplay.GetScissorRect());
//    //context.TransitionResource(GFX_RESOURCE(BlurTex5), RhiResourceState::GenericRead);
//    context.TransitionResource(GFX_RESOURCE(BlurTex6), RhiResourceState::RenderTarget);
//
//    GaussianHorizontalPass(context, GFX_SRV(BlurTex5), GFX_RTV(BlurTex6));
//    //context.SetViewport(viewport);
//    //context.SetScissor(gfxDisplay.GetScissorRect());
//    //GaussianVerticalPass(context, GFX_SRV(BlurTex6), GFX_RTV(BlurTex6));
//
//
//    // Final bloom pass
//    context.SetViewport(gfxDisplay.GetViewport());
//    context.SetScissor(gfxDisplay.GetScissorRect());
//    //context.TransitionResource(GFX_RESOURCE(BlurTex6), RhiResourceState::GenericRead);
//    //context.TransitionResource(GFX_RESOURCE(DeferredLightingOutput), RhiResourceState::GenericRead);
//
//    context.GetCommandList()->SetGraphicRootSignature(m_RootSignature2);
//    context.GetCommandList()->SetPrimitiveTopology(RhiPrimitiveTopology::TriangleStrip);
//    context.GetCommandList()->SetStencilRef(255);
//    context.GetCommandList()->SetDescriptorHeaps({ 1, &GraphicCore::GetSRVDescriptorHeap() });
//    context.GetCommandList()->SetRootConstantBuffer({ 0, GFX_RESOURCE(GlobalCommonConstants) });
//
//    context.SetRenderTarget(gfxDisplay.GetCurrentBackBufferRtv());
//    context.GetCommandList()->SetPipelineState(m_PipelineStateBloomComposite);
//    //context.GetCommandList()->SetRootDescriptorTable({ 1, gfxDisplay.GetCurrentBackBufferSrv() });
//    context.GetCommandList()->SetRootDescriptorTable({ 2, GFX_SRV(BlurTex1) });
//    context.GetCommandList()->SetRootDescriptorTable({ 3, GFX_SRV(BlurTex2) });
//    context.GetCommandList()->SetRootDescriptorTable({ 4, GFX_SRV(BlurTex3) });
//    context.GetCommandList()->SetRootDescriptorTable({ 5, GFX_SRV(BlurTex4) });
//    context.GetCommandList()->SetRootDescriptorTable({ 6, GFX_SRV(BlurTex5) });
//    context.GetCommandList()->SetRootDescriptorTable({ 7, GFX_SRV(BlurTex6) });
//    context.GetCommandList()->DrawInstanced({ 4, 1, 0, 0 });
//
//    context.FinalizeAndExecute();
//    context.Reset();
//}
//
//void BloomPass::GaussianHorizontalPass(GraphicContext& context, RhiShaderResourceViewHandle src, RhiRenderTargetViewHandle dest)
//{
//    context.GetCommandList()->SetGraphicRootSignature(m_RootSignature);
//    context.GetCommandList()->SetPrimitiveTopology(RhiPrimitiveTopology::TriangleStrip);
//    context.GetCommandList()->SetStencilRef(255);
//    context.GetCommandList()->SetDescriptorHeaps({ 1, &GraphicCore::GetSRVDescriptorHeap() });
//    context.GetCommandList()->SetRootConstantBuffer({ 0, GFX_RESOURCE(GlobalCommonConstants) });
//    context.SetRenderTarget(dest);
//    context.GetCommandList()->SetPipelineState(m_PipelineStateGaussianH);
//    context.GetCommandList()->SetRootDescriptorTable({ 1, src });
//    context.GetCommandList()->DrawInstanced({ 4, 1, 0, 0 });
//}
//
//void BloomPass::InitializeShaders()
//{
//    m_GaussianVS = std::make_unique<Shader>(L"common\\gaussian.hlsl", L"VS_Main", L"vs_6_0", ShaderType::Vertex);
//    m_GaussianHorizontalPS = std::make_unique<Shader>(L"common\\gaussian.hlsl", L"PS_Main_Horizontal", L"ps_6_0", ShaderType::Pixel);
//    m_BloomHighpassPS = std::make_unique<Shader>(L"bloom.hlsl", L"PS_HighPass", L"ps_6_0", ShaderType::Pixel);
//    m_BloomCompositePS = std::make_unique<Shader>(L"bloom.hlsl", L"PS_Composite", L"ps_6_0", ShaderType::Pixel);
//
//    m_GaussianVS->Compile();
//    m_GaussianHorizontalPS->Compile();
//    m_BloomHighpassPS->Compile();
//    m_BloomCompositePS->Compile();
//    m_GaussianVS->SetRecompiled(false);
//    m_GaussianHorizontalPS->SetRecompiled(false);
//    m_BloomHighpassPS->SetRecompiled(false);
//    m_BloomCompositePS->SetRecompiled(false);
//}
//
//void BloomPass::InitializePipelineState()
//{
//    m_PipelineStateGaussianH.SetName(L"BloomPass::GaussianH::PipelineState");
//
//    RhiPipelineState creationPSO;
//    creationPSO.SetBlendState(GraphicCore::GetGraphicCommon().m_BlendDisabled);
//    creationPSO.SetRasterizerState(GraphicCore::GetGraphicCommon().m_RasterizerDefault);
//    creationPSO.SetPrimitiveTopology(RhiPrimitiveTopologyType::Triangle);
//    creationPSO.SetVertexShader(m_GaussianVS->GetCompiledShader(), m_GaussianVS->GetCompiledShaderSize());
//    creationPSO.SetPixelShader(m_GaussianHorizontalPS->GetCompiledShader(), m_GaussianHorizontalPS->GetCompiledShaderSize());
//    creationPSO.SetInputLayout(GraphicCore::GetGraphicCommon().m_DefaultInputLayout);
//    creationPSO.SetRenderTargetFormat(BackBufferFormat);
//    creationPSO.SetDepthStencilState(GraphicCore::GetGraphicCommon().m_DepthStateReadOnly);
//    creationPSO.SetSamplingDesc(1, 0);
//    creationPSO.SetRootSignature(m_RootSignature);
//    creationPSO.Finalize(m_PipelineStateGaussianH);
//
//    creationPSO.SetPixelShader(m_BloomHighpassPS->GetCompiledShader(), m_BloomHighpassPS->GetCompiledShaderSize());
//    creationPSO.Finalize(m_PipelineStateBloomHighpass);
//
//    creationPSO.SetRootSignature(m_RootSignature2);
//    creationPSO.SetPixelShader(m_BloomCompositePS->GetCompiledShader(), m_BloomCompositePS->GetCompiledShaderSize());
//    creationPSO.Finalize(m_PipelineStateBloomComposite);
//
//}
//
//void BloomPass::InitializeRootSignature()
//{
//    m_RootSignature.SetName(L"BloomPass::RootSignature");
//
//    RhiRootSignature tempRS(2, 3);
//    tempRS.GetSampler(0) = GraphicCore::GetGraphicCommon().m_PointSampler;
//    tempRS.GetSampler(1) = GraphicCore::GetGraphicCommon().m_BilinearSampler;
//    tempRS.GetSampler(2) = GraphicCore::GetGraphicCommon().m_EnvMapSampler;
//    tempRS[0]->SetAsConstantBufferView({ 0, 0, RhiShaderVisibility::All });
//    tempRS[1]->SetAsDescriptorRange({ 0, 0, RhiShaderVisibility::Pixel, RhiDescriptorRangeType::Srv, 1 });
//    tempRS.Finalize(GraphicCore::GetGraphicCommon().m_DefaultRootSignatureFlags, m_RootSignature);
//
//    RhiRootSignature tempRS2(8, 3);
//    tempRS2.GetSampler(0) = GraphicCore::GetGraphicCommon().m_PointSampler;
//    tempRS2.GetSampler(1) = GraphicCore::GetGraphicCommon().m_BilinearSampler;
//    tempRS2.GetSampler(2) = GraphicCore::GetGraphicCommon().m_EnvMapSampler;
//    tempRS2[0]->SetAsConstantBufferView({ 0, 0, RhiShaderVisibility::All });
//    tempRS2[1]->SetAsDescriptorRange({ 0, 0, RhiShaderVisibility::Pixel, RhiDescriptorRangeType::Srv, 1 });
//    tempRS2[2]->SetAsDescriptorRange({ 1, 0, RhiShaderVisibility::Pixel, RhiDescriptorRangeType::Srv, 1 });
//    tempRS2[3]->SetAsDescriptorRange({ 2, 0, RhiShaderVisibility::Pixel, RhiDescriptorRangeType::Srv, 1 });
//    tempRS2[4]->SetAsDescriptorRange({ 3, 0, RhiShaderVisibility::Pixel, RhiDescriptorRangeType::Srv, 1 });
//    tempRS2[5]->SetAsDescriptorRange({ 4, 0, RhiShaderVisibility::Pixel, RhiDescriptorRangeType::Srv, 1 });
//    tempRS2[6]->SetAsDescriptorRange({ 5, 0, RhiShaderVisibility::Pixel, RhiDescriptorRangeType::Srv, 1 });
//    tempRS2[7]->SetAsDescriptorRange({ 6, 0, RhiShaderVisibility::Pixel, RhiDescriptorRangeType::Srv, 1 });
//    tempRS2.Finalize(GraphicCore::GetGraphicCommon().m_DefaultRootSignatureFlags, m_RootSignature2);
//}
//
//ETH_NAMESPACE_END
//
