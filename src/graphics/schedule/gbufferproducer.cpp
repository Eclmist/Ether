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

#include "graphics/schedule/gbufferproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/config/graphicconfig.h"

#include "graphics/resources/mesh.h"
#include "graphics/resources/material.h"

#include "graphics/shaders/common/globalconstants.h"

namespace Ether::Graphics::RhiLinkSpace
{
extern RhiGpuAddress g_GlobalConstantsCbv;
}

void Ether::Graphics::GBufferProducer::Initialize(ResourceContext& resourceContext)
{
    for (int i = 0; i < MaxSwapChainBuffers; ++i)
        m_FrameLocalUploadBuffer[i] = std::make_unique<UploadBufferAllocator>(_2MiB);

    RhiDevice& gfxDevice = GraphicCore::GetDevice();
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();

    m_VertexShader = gfxDevice.CreateShader({ "default.hlsl", "VS_Main", RhiShaderType::Vertex });
    m_PixelShader = gfxDevice.CreateShader({ "default.hlsl", "PS_Main", RhiShaderType::Pixel });

    if (GraphicCore::GetGraphicConfig().GetUseShaderDaemon())
    {
        GraphicCore::GetShaderDaemon().RegisterShader(*m_VertexShader);
        GraphicCore::GetShaderDaemon().RegisterShader(*m_PixelShader);
    }

    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(2, 0);
    rsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All);
    rsDesc->SetAsConstantBufferView(1, 1, RhiShaderVisibility::All);
    rsDesc->SetFlags(RhiRootSignatureFlag::AllowIAInputLayout);
    m_RootSignature = rsDesc->Compile("GBufferProducer Root Signature");

    m_PsoDesc = GraphicCore::GetDevice().CreatePipelineStateDesc();
    m_PsoDesc->SetVertexShader(*m_VertexShader);
    m_PsoDesc->SetPixelShader(*m_PixelShader);
    m_PsoDesc->SetRenderTargetFormat(BackBufferFormat);
    m_PsoDesc->SetRootSignature(*m_RootSignature);
    m_PsoDesc->SetInputLayout(
        VertexFormats::PositionNormalTangentBitangentTexcoord::s_InputElementDesc,
        VertexFormats::PositionNormalTangentBitangentTexcoord::s_NumElements);
    m_PsoDesc->SetDepthTargetFormat(DepthBufferFormat);
    m_PsoDesc->SetDepthStencilState(GraphicCore::GetGraphicCommon().m_DepthStateReadWrite);

    resourceContext.RegisterPipelineState("GBufferProducer Pipeline State", * m_PsoDesc);
}

void Ether::Graphics::GBufferProducer::FrameSetup(ResourceContext& resourceContext)
{
    static RhiClearValue clearValue = { DepthBufferFormat, { 1.0, 0 } };
    static RhiCommitedResourceDesc prevDesc = {};

    RhiCommitedResourceDesc desc = {};
    desc.m_HeapType = RhiHeapType::Default;
    desc.m_State = RhiResourceState::DepthWrite;
    desc.m_ClearValue = &clearValue;
    desc.m_ResourceDesc = RhiCreateDepthStencilResourceDesc(
        DepthBufferFormat,
        GraphicCore::GetGraphicConfig().GetResolution());
    desc.m_Name = "Depth Buffer";

    if (std::memcmp(&prevDesc.m_ResourceDesc, &desc.m_ResourceDesc, sizeof(RhiResourceDesc)) == 0)
        return;

    prevDesc = desc;

    m_DepthBuffer = GraphicCore::GetDevice().CreateCommittedResource(desc);

    auto alloc = GraphicCore::GetDsvAllocator().Allocate();

    static RhiDepthStencilViewDesc dsvDesc = {};
    dsvDesc.m_Format = DepthBufferFormat;
    dsvDesc.m_Resource = m_DepthBuffer.get();
    dsvDesc.m_TargetCpuAddress = ((DescriptorAllocation&)(*alloc)).GetCpuAddress();

    m_Dsv = GraphicCore::GetDevice().CreateDepthStencilView(dsvDesc);
    GraphicCore::GetDsvAllocator().Free(std::move(alloc));
}

void Ether::Graphics::GBufferProducer::Render(GraphicContext& graphicContext, ResourceContext& resourceContext)
{
    ETH_MARKER_EVENT("Temp Frame Dump - Render");
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();

    graphicContext.PushMarker("Clear");
    graphicContext.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::RenderTarget);
    graphicContext.ClearColor(gfxDisplay.GetBackBufferRtv(), config.GetClearColor());
    graphicContext.ClearDepthStencil(*m_Dsv, 1.0);
    graphicContext.PopMarker();

    graphicContext.PushMarker("Draw GBuffer Geometry");
    graphicContext.SetViewport(gfxDisplay.GetViewport());
    graphicContext.SetScissorRect(gfxDisplay.GetScissorRect());
    graphicContext.SetPrimitiveTopology(RhiPrimitiveTopology::TriangleList);
    graphicContext.SetDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
    graphicContext.SetGraphicRootSignature(*m_RootSignature);
    graphicContext.SetComputeRootSignature(*m_RootSignature);
    graphicContext.SetPipelineState(resourceContext.GetPipelineState(*m_PsoDesc));
    graphicContext.SetGraphicsRootConstantBufferView(0, RhiLinkSpace::g_GlobalConstantsCbv);
    graphicContext.SetComputeRootConstantBufferView(0, RhiLinkSpace::g_GlobalConstantsCbv);

    graphicContext.SetRenderTarget(gfxDisplay.GetBackBufferRtv(), m_Dsv.get());

    const VisualBatch& visualBatch = graphicContext.GetVisualBatch();

    for (int i = 0; i < visualBatch.m_Visuals.size(); ++i)
    {
        ETH_MARKER_EVENT("Draw Meshes");
        Visual visual = visualBatch.m_Visuals[i];

        auto alloc = m_FrameLocalUploadBuffer[gfxDisplay.GetBackBufferIndex()]->Allocate({ sizeof(Material), 256 });
        memcpy(alloc->GetCpuHandle(), visual.m_Material, sizeof(Material));

        graphicContext.SetGraphicsRootConstantBufferView(
            1,
            dynamic_cast<UploadBufferAllocation&>(*alloc).GetGpuAddress());
        graphicContext.SetVertexBuffer(visual.m_Mesh->GetVertexBufferView());
        graphicContext.SetIndexBuffer(visual.m_Mesh->GetIndexBufferView());
        graphicContext.DrawIndexedInstanced(visual.m_Mesh->GetNumIndices(), 1);
    }

    graphicContext.PopMarker();
}

void Ether::Graphics::GBufferProducer::Reset()
{
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    m_FrameLocalUploadBuffer[gfxDisplay.GetBackBufferIndex()]->Reset();
}

