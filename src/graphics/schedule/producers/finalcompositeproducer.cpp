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

#include "finalcompositeproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/shaders/common/globalconstants.h"

DEFINE_GFX_PA(FinalCompositeProducer)

DECLARE_GFX_SR(PostFxSourceTexture)
DECLARE_GFX_CB(GlobalRingBuffer)

Ether::Graphics::FinalCompositeProducer::FinalCompositeProducer()
    : FullScreenProducer("FinalCompositeProducer", "finalcomposite.hlsl")
{
}

void Ether::Graphics::FinalCompositeProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    schedule.Read(ACCESS_GFX_SR(PostFxSourceTexture));
    schedule.Read(ACCESS_GFX_CB(GlobalRingBuffer));
}

void Ether::Graphics::FinalCompositeProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    FullScreenProducer::RenderFrame(ctx, rc);

    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_SR(PostFxSourceTexture)), RhiResourceState::Common);
    ctx.TransitionResource(GraphicCore::GetGraphicDisplay().GetBackBuffer(), RhiResourceState::RenderTarget);
    ctx.SetGraphicsRootDescriptorTable(1, ACCESS_GFX_SR(PostFxSourceTexture)->GetGpuAddress());
    ctx.SetRenderTarget(GraphicCore::GetGraphicDisplay().GetBackBufferRtv());
    ctx.DrawInstanced(3, 1);
}

void Ether::Graphics::FinalCompositeProducer::CreatePipelineState(ResourceContext& rc)
{
    m_PsoDesc = GraphicCore::GetDevice().CreateGraphicPipelineStateDesc();
    m_PsoDesc->SetVertexShader(*m_VertexShader);
    m_PsoDesc->SetPixelShader(*m_PixelShader);
    m_PsoDesc->SetRenderTargetFormat(BackBufferFormat);
    m_PsoDesc->SetRootSignature(*m_RootSignature);
    m_PsoDesc->SetInputLayout(nullptr, 0);
    m_PsoDesc->SetDepthStencilState(GraphicCore::GetGraphicCommon().m_DepthStateDisabled);
    rc.RegisterPipelineState((GetName() + " Pipeline State").c_str(), *m_PsoDesc);
}

void Ether::Graphics::FinalCompositeProducer::CreateRootSignature()
{
    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(2, 0);
    rsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All);     // (b0) Global Constants
    rsDesc->SetAsDescriptorTable(1, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(1, RhiDescriptorType::Srv, 1, 0, 0); // (t0) LightingCompositeTexture
    rsDesc->SetFlags(RhiRootSignatureFlag::DirectlyIndexed);
    m_RootSignature = rsDesc->Compile((GetName() + " Root Signature").c_str());
}

