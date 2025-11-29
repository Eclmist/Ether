/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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

#include "fullscreenpixelproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/shaders/common/globalconstants.h"

DECLARE_GFX_CB(GlobalConstants)

Ether::Graphics::FullScreenPixelProducer::FullScreenPixelProducer(const char* name, const char* shaderPath)
    : GraphicProducer(name)
    , m_ShaderPath(shaderPath)
{
}

void Ether::Graphics::FullScreenPixelProducer::Initialize(ResourceContext& rc)
{
    CreateShaders();
    CreateRootSignature();
    CreatePipelineState(rc);
}

void Ether::Graphics::FullScreenPixelProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    ETH_MARKER_EVENT("FullScreenProducer");

    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();
    
    ctx.SetViewport(gfxDisplay.GetViewport());
    ctx.SetScissorRect(gfxDisplay.GetScissorRect());
    ctx.SetPrimitiveTopology(RhiPrimitiveTopology::TriangleList);
    ctx.SetSrvCbvUavDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
    ctx.SetSamplerDescriptorHeap(GraphicCore::GetSamplerAllocator().GetDescriptorHeap());
    ctx.SetGraphicRootSignature(*m_RootSignature);
    ctx.SetGraphicPipelineState((RhiGraphicPipelineState&)rc.GetPipelineState(*m_PsoDesc));
    ctx.Bind(ACCESS_GFX_CB(GlobalConstants), GetRingBufferOffset());
}

void Ether::Graphics::FullScreenPixelProducer::CreateShaders()
{
    RhiDevice& gfxDevice = GraphicCore::GetDevice();
    m_VertexShader = gfxDevice.CreateShader({ "fullscreen_vs.hlsl", "VS_Main", RhiShaderType::Vertex });
    m_PixelShader = gfxDevice.CreateShader({ m_ShaderPath.c_str(), "PS_Main", RhiShaderType::Pixel });

    m_VertexShader->Compile();
    m_PixelShader->Compile();

    GraphicCore::GetShaderDaemon().RegisterShader(*m_VertexShader);
    GraphicCore::GetShaderDaemon().RegisterShader(*m_PixelShader);
}

void Ether::Graphics::FullScreenPixelProducer::CreateRootSignature()
{
    m_RootSignature = GraphicCore::GetDevice().CreateRootSignatureDesc(m_PixelShader->GetReflection())->Compile((GetName() + " Root Signature").c_str());
}

void Ether::Graphics::FullScreenPixelProducer::CreatePipelineState(ResourceContext& rc)
{
    m_PsoDesc = GraphicCore::GetDevice().CreateGraphicPipelineStateDesc();
    m_PsoDesc->SetVertexShader(*m_VertexShader);
    m_PsoDesc->SetPixelShader(*m_PixelShader);
    m_PsoDesc->SetRenderTargetFormat(BackBufferHdrFormat);
    m_PsoDesc->SetRootSignature(*m_RootSignature);
    m_PsoDesc->SetInputLayout(nullptr, 0);
    m_PsoDesc->SetDepthStencilState(GraphicCore::GetGraphicCommon().m_DepthStateDisabled);
    rc.RegisterPipelineState((GetName() + " Pipeline State").c_str(), *m_PsoDesc);
}

void Ether::Graphics::FullScreenPixelProducer::DrawFullScreen(GraphicContext& ctx)
{
    ctx.DrawInstanced(3, 1);
}

