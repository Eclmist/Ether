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

#ifdef ETH_GRAPHICS_DX12

#include "graphics/graphiccore.h"
#include "graphics/rhi/dx12/dx12pipelinestate.h"
#include "graphics/rhi/dx12/dx12translation.h"
#include "graphics/rhi/dx12/dx12rootsignature.h"

Ether::Graphics::Dx12PipelineStateDesc::Dx12PipelineStateDesc()
    : RhiPipelineStateDesc()
    , m_Dx12PsoDesc{}
{
    SetBlendState(GraphicCore::GetGraphicCommon().m_BlendDisabled);
    SetRasterizerState(GraphicCore::GetGraphicCommon().m_RasterizerDefaultCw);
    SetDepthStencilState(GraphicCore::GetGraphicCommon().m_DepthStateDisabled);
    SetPrimitiveTopology(RhiPrimitiveTopologyType::Triangle);
    SetSamplingDesc(1, 0);
    SetSampleMask(0xFFFFFFFF);
    SetNodeMask(0);
}

void Ether::Graphics::Dx12PipelineStateDesc::SetBlendState(RhiBlendDesc desc)
{
    m_Dx12PsoDesc.BlendState = Translate(desc);
}

void Ether::Graphics::Dx12PipelineStateDesc::SetRasterizerState(RhiRasterizerDesc desc)
{
    m_Dx12PsoDesc.RasterizerState = Translate(desc);
}

void Ether::Graphics::Dx12PipelineStateDesc::SetInputLayout(RhiInputLayoutDesc desc)
{
    for (int i = 0; i < desc.m_NumElements; ++i)
        m_InputElements.push_back(Translate(desc.m_InputElementDescs[i]));

    m_Dx12PsoDesc.InputLayout.NumElements = desc.m_NumElements;
    m_Dx12PsoDesc.InputLayout.pInputElementDescs = m_InputElements.data();
}

void Ether::Graphics::Dx12PipelineStateDesc::SetPrimitiveTopology(RhiPrimitiveTopologyType type)
{
    m_Dx12PsoDesc.PrimitiveTopologyType = Translate(type);
}

void Ether::Graphics::Dx12PipelineStateDesc::SetDepthStencilState(RhiDepthStencilDesc desc)
{
    m_Dx12PsoDesc.DepthStencilState = Translate(desc);
}

void Ether::Graphics::Dx12PipelineStateDesc::SetDepthTargetFormat(RhiFormat dsvFormat)
{
    m_Dx12PsoDesc.DSVFormat = Translate(dsvFormat);
}

void Ether::Graphics::Dx12PipelineStateDesc::SetRenderTargetFormat(RhiFormat rtvFormat)
{
    return SetRenderTargetFormats(1, &rtvFormat);
}

void Ether::Graphics::Dx12PipelineStateDesc::SetRenderTargetFormats(uint32_t numRtv, const RhiFormat* rtvFormats)
{
    m_Dx12PsoDesc.NumRenderTargets = numRtv;
    for (int i = 0; i < numRtv; ++i)
        m_Dx12PsoDesc.RTVFormats[i] = Translate(rtvFormats[i]);
}

void Ether::Graphics::Dx12PipelineStateDesc::SetRootSignature(const RhiRootSignature& rootSignature)
{
    m_Dx12PsoDesc.pRootSignature = static_cast<const Dx12RootSignature&>(rootSignature).m_RootSignature.Get();
}

void Ether::Graphics::Dx12PipelineStateDesc::SetSamplingDesc(uint32_t numMsaaSamples, uint32_t msaaQuality)
{
    m_Dx12PsoDesc.SampleDesc.Count = numMsaaSamples;
    m_Dx12PsoDesc.SampleDesc.Quality = msaaQuality;
}

void Ether::Graphics::Dx12PipelineStateDesc::SetVertexShader(RhiShader& vs)
{
    AssertGraphics(vs.GetType() == RhiShaderType::Vertex, "Vertex shader expected, but encountered %u", (uint32_t)vs.GetType());
    m_Dx12PsoDesc.VS.pShaderBytecode = vs.GetCompiledData();
    m_Dx12PsoDesc.VS.BytecodeLength = vs.GetCompiledSize();
    m_Shaders[vs.GetType()] = &vs;
}

void Ether::Graphics::Dx12PipelineStateDesc::SetPixelShader(RhiShader& ps)
{
    AssertGraphics(ps.GetType() == RhiShaderType::Pixel, "Pixel shader expected, but encountered %u", (uint32_t)ps.GetType());
    m_Dx12PsoDesc.PS.pShaderBytecode = ps.GetCompiledData();
    m_Dx12PsoDesc.PS.BytecodeLength = ps.GetCompiledSize();
    m_Shaders[ps.GetType()] = &ps;
}

void Ether::Graphics::Dx12PipelineStateDesc::SetNodeMask(uint32_t mask)
{
    m_Dx12PsoDesc.NodeMask = mask;
}

void Ether::Graphics::Dx12PipelineStateDesc::SetSampleMask(uint32_t mask)
{
    m_Dx12PsoDesc.SampleMask = mask;
}

void Ether::Graphics::Dx12PipelineStateDesc::Reset()
{
    m_Dx12PsoDesc = {};
}

#endif // ETH_GRAPHICS_DX12

