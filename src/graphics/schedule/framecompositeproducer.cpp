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

#include "graphics/schedule/framecompositeproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/shaders/common/framecompositeinputs.h"

namespace Ether::Graphics::RhiLinkSpace
{
extern RhiGpuAddress g_GlobalConstantsCbv;
}

void Ether::Graphics::FrameCompositeProducer::Initialize(ResourceContext& rc)
{
    for (int i = 0; i < MaxSwapChainBuffers; ++i)
        m_FrameLocalUploadBuffer[i] = std::make_unique<UploadBufferAllocator>(_2MiB);

    CreateShaders();
    CreateRootSignature();
    CreatePipelineState(rc);
}

void Ether::Graphics::FrameCompositeProducer::FrameSetup(ResourceContext& rc)
{
}

void Ether::Graphics::FrameCompositeProducer::Render(GraphicContext& ctx, ResourceContext& rc)
{
    ETH_MARKER_EVENT("Temp Frame Dump - Render");
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();
    
    Shader::FrameCompositeInputs cbv;
    cbv.m_GBufferTextureIndex0 = GraphicCore::GetBindlessDescriptorManager().GetDescriptorIndex("GBuffer - SRV 0");
    cbv.m_GBufferTextureIndex1 = GraphicCore::GetBindlessDescriptorManager().GetDescriptorIndex("GBuffer - SRV 1");
    cbv.m_GBufferTextureIndex2 = GraphicCore::GetBindlessDescriptorManager().GetDescriptorIndex("GBuffer - SRV 2");
    cbv.m_LightingTextureIndex = GraphicCore::GetBindlessDescriptorManager().GetDescriptorIndex("Raytrace - Output UAV");

    auto alloc = m_FrameLocalUploadBuffer[GraphicCore::GetGraphicDisplay().GetBackBufferIndex()]
        ->Allocate({ sizeof(Shader::FrameCompositeInputs), 256 });
    memcpy(alloc->GetCpuHandle(), &cbv, sizeof(Shader::FrameCompositeInputs));

    ctx.SetViewport(gfxDisplay.GetViewport());
    ctx.SetScissorRect(gfxDisplay.GetScissorRect());
    ctx.SetPrimitiveTopology(RhiPrimitiveTopology::TriangleList);
    ctx.SetDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
    ctx.SetGraphicRootSignature(*m_RootSignature);
    ctx.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::RenderTarget);
    ctx.SetPipelineState(rc.GetPipelineState(*m_PsoDesc));
    ctx.SetGraphicsRootConstantBufferView(0, RhiLinkSpace::g_GlobalConstantsCbv);
    ctx.SetGraphicsRootConstantBufferView(1, dynamic_cast<UploadBufferAllocation&>(*alloc).GetGpuAddress());
    ctx.SetRenderTarget(gfxDisplay.GetBackBufferRtv());
    ctx.DrawInstanced(3, 1);
}

void Ether::Graphics::FrameCompositeProducer::Reset()
{
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    m_FrameLocalUploadBuffer[gfxDisplay.GetBackBufferIndex()]->Reset();
}

void Ether::Graphics::FrameCompositeProducer::CreateShaders()
{
    RhiDevice& gfxDevice = GraphicCore::GetDevice();
    m_VertexShader = gfxDevice.CreateShader({ "framecomposite.hlsl", "VS_Main", RhiShaderType::Vertex });
    m_PixelShader = gfxDevice.CreateShader({ "framecomposite.hlsl", "PS_Main", RhiShaderType::Pixel });

    if (GraphicCore::GetGraphicConfig().GetUseShaderDaemon())
    {
        GraphicCore::GetShaderDaemon().RegisterShader(*m_VertexShader);
        GraphicCore::GetShaderDaemon().RegisterShader(*m_PixelShader);
    }
}

void Ether::Graphics::FrameCompositeProducer::CreateRootSignature()
{
    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(2, 1);
    rsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All);
    rsDesc->SetAsConstantBufferView(1, 1, RhiShaderVisibility::All);
    rsDesc->SetAsSampler(0, GraphicCore::GetGraphicCommon().m_BilinearSampler, RhiShaderVisibility::All);
    rsDesc->SetFlags(RhiRootSignatureFlag::DirectlyIndexed);
    m_RootSignature = rsDesc->Compile("FrameComposite Root Signature");
}

void Ether::Graphics::FrameCompositeProducer::CreatePipelineState(ResourceContext& rc)
{
    m_PsoDesc = GraphicCore::GetDevice().CreatePipelineStateDesc();
    m_PsoDesc->SetVertexShader(*m_VertexShader);
    m_PsoDesc->SetPixelShader(*m_PixelShader);
    m_PsoDesc->SetRenderTargetFormat(RhiFormat::R8G8B8A8Unorm);
    m_PsoDesc->SetRootSignature(*m_RootSignature);
    m_PsoDesc->SetInputLayout(nullptr, 0);
    m_PsoDesc->SetDepthStencilState(GraphicCore::GetGraphicCommon().m_DepthStateDisabled);
    rc.RegisterPipelineState("FrameCompositeProducer Pipeline State", *m_PsoDesc);
}
