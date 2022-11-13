///*
//    This file is part of Ether, an open-source DirectX 12 renderer.
//
//    Copyright (c) 2020-2023 Samuel Huang - All rights reserved.
//
//    Ether is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program. If not, see <http://www.gnu.org/licenses/>.
//*/
//
//#include "ProceduralSkyPass.h"
//#include "graphic/rhi/rhicommandlist.h"
//#include "graphic/rhi/rhicommandqueue.h"
//#include "graphic/rhi/rhipipelinestate.h"
//#include "graphic/rhi/rhirootparameter.h"
//#include "graphic/rhi/rhirootsignature.h"
//
//ETH_NAMESPACE_BEGIN
//
//DEFINE_GFX_PASS(ProceduralSkyPass);
//DECLARE_GFX_RESOURCE(GlobalCommonConstants);
//
//ProceduralSkyPass::ProceduralSkyPass()
//    : RenderPass("Procedural Sky Pass")
//{
//}
//
//void ProceduralSkyPass::Initialize()
//{
//    InitializeShaders();
//    InitializeRootSignature();
//    InitializePipelineState();
//}
//
//void ProceduralSkyPass::RegisterInputOutput(GraphicContext& context, ResourceContext& rc)
//{
//}
//
//void ProceduralSkyPass::Render(GraphicContext& context, ResourceContext& rc)
//{
//    // TODO: Properly support shader hot reload - each PSO should check their own shaders
//    if (m_VertexShader->HasRecompiled() || m_PixelShader->HasRecompiled())
//    {
//        context.GetCommandQueue()->Flush();
//        InitializePipelineState();
//        m_VertexShader->SetRecompiled(false);
//        m_PixelShader->SetRecompiled(false);
//    }
//
//    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
//
//    context.SetRenderTarget(gfxDisplay.GetCurrentBackBufferRtv());
//    context.SetViewport(gfxDisplay.GetViewport());
//    context.SetScissor(gfxDisplay.GetScissorRect());
//
//    context.GetCommandList()->SetPipelineState(m_PipelineState);
//    context.GetCommandList()->SetGraphicRootSignature(m_RootSignature);
//    context.GetCommandList()->SetPrimitiveTopology(RhiPrimitiveTopology::TriangleStrip);
//    context.GetCommandList()->SetDescriptorHeaps({ 1, &GraphicCore::GetGpuDescriptorAllocator().GetDescriptorHeap() });
//    context.GetCommandList()->SetRootConstantBuffer({ 0, GFX_RESOURCE(GlobalCommonConstants) });
//
//    auto texture = GraphicCore::GetGraphicRenderer().m_EnvironmentHDRI;
//    if (texture != nullptr)
//    {
//        rc.InitializeTexture2D(*texture);
//        context.GetCommandList()->SetRootDescriptorTable({ 1, texture->GetView()});
//    }
//
//    context.GetCommandList()->DrawInstanced({ 4, 1, 0, 0 });
//
//    context.FinalizeAndExecute();
//    context.Reset();
//}
//
//void ProceduralSkyPass::InitializeShaders()
//{
//    m_VertexShader = std::make_unique<Shader>(L"proceduralsky.hlsl", L"VS_Main", L"vs_6_0", ShaderType::Vertex);
//    m_PixelShader = std::make_unique<Shader>(L"proceduralsky.hlsl", L"PS_Main", L"ps_6_0", ShaderType::Pixel);
//
//    m_VertexShader->Compile();
//    m_PixelShader->Compile();
//    m_VertexShader->SetRecompiled(false);
//    m_PixelShader->SetRecompiled(false);
//}
//
//void ProceduralSkyPass::InitializePipelineState()
//{
//    m_PipelineState.SetName(L"ProceduralSkyPass::PipelineState");
//
//    RhiPipelineState creationPSO;
//    creationPSO.SetBlendState(GraphicCore::GetGraphicCommon().m_BlendDisabled);
//    creationPSO.SetRasterizerState(GraphicCore::GetGraphicCommon().m_RasterizerDefault);
//    creationPSO.SetPrimitiveTopology(RhiPrimitiveTopologyType::Triangle);
//    creationPSO.SetVertexShader(m_VertexShader->GetCompiledShader(), m_VertexShader->GetCompiledShaderSize());
//    creationPSO.SetPixelShader(m_PixelShader->GetCompiledShader(), m_PixelShader->GetCompiledShaderSize());
//    creationPSO.SetInputLayout(GraphicCore::GetGraphicCommon().m_DefaultInputLayout);
//    creationPSO.SetRenderTargetFormat(BackBufferFormat);
//    creationPSO.SetDepthStencilState(GraphicCore::GetGraphicCommon().m_DepthStateDisabled);
//    creationPSO.SetSamplingDesc(1, 0);
//    creationPSO.SetRootSignature(m_RootSignature);
//    creationPSO.Finalize(m_PipelineState);
//}
//
//void ProceduralSkyPass::InitializeRootSignature()
//{
//    m_RootSignature.SetName(L"ProceduralSkyPass::RootSignature");
//    RhiRootSignature tempRS(2, 3);
//    tempRS.GetSampler(0) = GraphicCore::GetGraphicCommon().m_PointSampler;
//    tempRS.GetSampler(1) = GraphicCore::GetGraphicCommon().m_BilinearSampler;
//    tempRS.GetSampler(2) = GraphicCore::GetGraphicCommon().m_EnvMapSampler;
//    tempRS[0]->SetAsConstantBufferView({ 0, 0, RhiShaderVisibility::All });
//    tempRS[1]->SetAsDescriptorRange({ 0, 0, RhiShaderVisibility::Pixel, RhiDescriptorRangeType::Srv, 1 });    // Albedo
//    tempRS.Finalize(GraphicCore::GetGraphicCommon().m_DefaultRootSignatureFlags, m_RootSignature);
//}
//
//ETH_NAMESPACE_END
//
