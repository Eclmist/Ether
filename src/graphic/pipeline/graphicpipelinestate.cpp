/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel Van Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "graphicpipelinestate.h"

ETH_NAMESPACE_BEGIN

GraphicPipelineState::GraphicPipelineState(const std::wstring& name)
    : PipelineState(name)
{
    m_Desc = {};
}

void GraphicPipelineState::SetBlendState(const D3D12_BLEND_DESC& desc)
{
    m_Desc.BlendState = desc;
}

void GraphicPipelineState::SetRasterizerState(const D3D12_RASTERIZER_DESC& desc)
{
    m_Desc.RasterizerState = desc;
}

void GraphicPipelineState::SetInputLayout(const D3D12_INPUT_LAYOUT_DESC& desc)
{
    m_Desc.InputLayout = desc;
}

void GraphicPipelineState::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE type)
{
    m_Desc.PrimitiveTopologyType = type;
}

void GraphicPipelineState::SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& desc)
{
    m_Desc.DepthStencilState = desc;
}

void GraphicPipelineState::SetDepthTargetFormat(DXGI_FORMAT dsvFormat)
{
    m_Desc.DSVFormat = dsvFormat;
}

void GraphicPipelineState::SetRenderTargetFormat(DXGI_FORMAT rtvFormat)
{
    SetRenderTargetFormats(1, &rtvFormat);
}

void GraphicPipelineState::SetRenderTargetFormats(uint32_t numRtv, const DXGI_FORMAT* rtvFormats)
{
    m_Desc.NumRenderTargets = numRtv;

    for (uint32_t i = 0; i < numRtv; ++i)
        m_Desc.RTVFormats[i] = rtvFormats[i];
}

void GraphicPipelineState::SetSamplingDesc(uint32_t numMsaaSamples, uint32_t msaaQuality)
{
    m_Desc.SampleDesc.Count = numMsaaSamples;
    m_Desc.SampleDesc.Quality = msaaQuality;
}

void GraphicPipelineState::SetSampleMask(uint32_t sampleMask)
{
    m_Desc.SampleMask = sampleMask;
}

void GraphicPipelineState::SetVertexShader(const void* binary, size_t size)
{
    m_Desc.VS.pShaderBytecode = binary;
    m_Desc.VS.BytecodeLength = size;
}

void GraphicPipelineState::SetPixelShader(const void* binary, size_t size)
{
    m_Desc.PS.pShaderBytecode = binary;
    m_Desc.PS.BytecodeLength = size;
}

void GraphicPipelineState::Finalize()
{
    m_Desc.pRootSignature = m_RootSignature->GetRootSignature();

    ASSERT_SUCCESS(GraphicCore::GetDevice().CreateGraphicsPipelineState(&m_Desc, IID_PPV_ARGS(&m_PipelineState)));
    m_PipelineState->SetName(m_Name.c_str());
}

ETH_NAMESPACE_END

