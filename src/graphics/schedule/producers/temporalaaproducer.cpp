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
//#include "finalcompositeproducer.h"
//
//#include "graphics/graphiccore.h"
//#include "graphics/shaders/common/globalconstants.h"
//
//DEFINE_GFX_PA(FinalCompositeProducer)
//
//DECLARE_GFX_SR(GBufferTexture0)
//DECLARE_GFX_SR(GBufferTexture1)
//DECLARE_GFX_SR(GBufferTexture2)
//DECLARE_GFX_SR(RTLightingTexture)
//DECLARE_GFX_CB(GlobalRingBuffer)
//
//void Ether::Graphics::FinalCompositeProducer::Initialize(ResourceContext& rc)
//{
//    CreateShaders();
//    CreateRootSignature();
//    CreatePipelineState(rc);
//}
//
//void Ether::Graphics::FinalCompositeProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
//{
//    schedule.Read(ACCESS_GFX_SR(GBufferTexture0));
//    schedule.Read(ACCESS_GFX_SR(GBufferTexture1));
//    schedule.Read(ACCESS_GFX_SR(GBufferTexture2));
//    schedule.Read(ACCESS_GFX_SR(RTLightingTexture));
//    schedule.Read(ACCESS_GFX_CB(GlobalRingBuffer));
//}
//
//void Ether::Graphics::FinalCompositeProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
//{
//    ETH_MARKER_EVENT("FinalCompositeProducer - Render");
//    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
//    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
//    const GraphicConfig& config = GraphicCore::GetGraphicConfig();
//    
//    ctx.SetViewport(gfxDisplay.GetViewport());
//    ctx.SetScissorRect(gfxDisplay.GetScissorRect());
//    ctx.SetPrimitiveTopology(RhiPrimitiveTopology::TriangleList);
//    ctx.SetSrvCbvUavDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
//    ctx.SetSamplerDescriptorHeap(GraphicCore::GetSamplerAllocator().GetDescriptorHeap());
//    ctx.SetGraphicRootSignature(*m_RootSignature);
//    ctx.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::RenderTarget);
//    ctx.SetPipelineState(rc.GetPipelineState(*m_PsoDesc));
//
//    uint64_t ringBufferOffset = gfxDisplay.GetBackBufferIndex() * sizeof(Shader::GlobalConstants);
//    ctx.SetGraphicsRootConstantBufferView(0, rc.GetResource(ACCESS_GFX_CB(GlobalRingBuffer))->GetGpuAddress() + ringBufferOffset);
//    ctx.SetGraphicsRootDescriptorTable(1, ACCESS_GFX_SR(GBufferTexture0)->GetGpuAddress());
//    ctx.SetGraphicsRootDescriptorTable(2, ACCESS_GFX_SR(GBufferTexture1)->GetGpuAddress());
//    ctx.SetGraphicsRootDescriptorTable(3, ACCESS_GFX_SR(GBufferTexture2)->GetGpuAddress());
//    ctx.SetGraphicsRootDescriptorTable(4, ACCESS_GFX_SR(RTLightingTexture)->GetGpuAddress());
//
//    ctx.SetRenderTarget(gfxDisplay.GetBackBufferRtv());
//    ctx.DrawInstanced(3, 1);
//}
//
//void Ether::Graphics::FinalCompositeProducer::CreateShaders()
//{
//    RhiDevice& gfxDevice = GraphicCore::GetDevice();
//    m_VertexShader = gfxDevice.CreateShader({ "finalcomposite.hlsl", "VS_Main", RhiShaderType::Vertex });
//    m_PixelShader = gfxDevice.CreateShader({ "finalcomposite.hlsl", "PS_Main", RhiShaderType::Pixel });
//
//    m_VertexShader->Compile();
//    m_PixelShader->Compile();
//
//    GraphicCore::GetShaderDaemon().RegisterShader(*m_VertexShader);
//    GraphicCore::GetShaderDaemon().RegisterShader(*m_PixelShader);
//}
//
//void Ether::Graphics::FinalCompositeProducer::CreateRootSignature()
//{
//    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(5, 0);
//    rsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All);     // (b0) Global Constants
//
//    rsDesc->SetAsDescriptorTable(1, 1, RhiShaderVisibility::All);
//    rsDesc->SetDescriptorTableRange(1, RhiDescriptorType::Srv, 1, 0, 0); // (t0) GBufferTexture0
//    rsDesc->SetAsDescriptorTable(2, 1, RhiShaderVisibility::All);
//    rsDesc->SetDescriptorTableRange(2, RhiDescriptorType::Srv, 1, 0, 1); // (t1) GBufferTexture1
//    rsDesc->SetAsDescriptorTable(3, 1, RhiShaderVisibility::All);
//    rsDesc->SetDescriptorTableRange(3, RhiDescriptorType::Srv, 1, 0, 2); // (t2) GBufferTexture2
//    rsDesc->SetAsDescriptorTable(4, 1, RhiShaderVisibility::All);
//    rsDesc->SetDescriptorTableRange(4, RhiDescriptorType::Srv, 1, 0, 3); // (t3) RTLightingTexture
//
//    rsDesc->SetFlags(RhiRootSignatureFlag::DirectlyIndexed);
//    m_RootSignature = rsDesc->Compile("FinalCompositeProducer Root Signature");
//}
//
//void Ether::Graphics::FinalCompositeProducer::CreatePipelineState(ResourceContext& rc)
//{
//    m_PsoDesc = GraphicCore::GetDevice().CreatePipelineStateDesc();
//    m_PsoDesc->SetVertexShader(*m_VertexShader);
//    m_PsoDesc->SetPixelShader(*m_PixelShader);
//    m_PsoDesc->SetRenderTargetFormat(RhiFormat::R8G8B8A8Unorm);
//    m_PsoDesc->SetRootSignature(*m_RootSignature);
//    m_PsoDesc->SetInputLayout(nullptr, 0);
//    m_PsoDesc->SetDepthStencilState(GraphicCore::GetGraphicCommon().m_DepthStateDisabled);
//    rc.RegisterPipelineState("FinalCompositeProducer Pipeline State", *m_PsoDesc);
//}
//
