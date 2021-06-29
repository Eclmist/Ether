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
}

void GraphicPipelineState::SetInputLayout(uint32_t numLayoutElements, const D3D12_INPUT_ELEMENT_DESC& desc)
{
    m_Desc.InputLayout.NumElements = numLayoutElements;
    // TODO
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
    for (uint32_t i = 0; i < numRtv; ++i)
        m_Desc.RTVFormats[i] = rtvFormats[i];

    for (uint32_t i = numRtv; i < m_Desc.NumRenderTargets; ++i)
        m_Desc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;

    m_Desc.NumRenderTargets = numRtv;
}

void GraphicPipelineState::SetSamplingDesc(uint32_t numMsaaSamples, uint32_t msaaQuality)
{
    m_Desc.SampleDesc.Count = numMsaaSamples;
    m_Desc.SampleDesc.Quality = msaaQuality;
}

void GraphicPipelineState::SetVertexShader(const void* binary, size_t size)
{
    m_Desc.VS = CD3DX12_SHADER_BYTECODE(binary, size);
}

void GraphicPipelineState::SetPixelShader(const void* binary, size_t size)
{
    m_Desc.PS = CD3DX12_SHADER_BYTECODE(binary, size);
}

void GraphicPipelineState::Finalize()
{
    //TODO: Initialize root signature

}


ETH_NAMESPACE_END