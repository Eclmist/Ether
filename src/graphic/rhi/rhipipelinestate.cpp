/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "rhipipelinestate.h"
#include "rhidevice.h"

ETH_NAMESPACE_BEGIN

RhiResult RhiPipelineState::SetBlendState(const RhiBlendDesc& desc)
{
    m_CreationDesc.m_BlendDesc = desc;
    return RhiResult::Success;
}

RhiResult RhiPipelineState::SetRasterizerState(const RhiRasterizerDesc& desc)
{
    m_CreationDesc.m_RasterizerDesc = desc;
    return RhiResult::Success;
}

RhiResult RhiPipelineState::SetInputLayout(const RhiInputLayoutDesc& desc)
{
    m_CreationDesc.m_InputLayoutDesc = desc;
    return RhiResult::Success;
}

RhiResult RhiPipelineState::SetPrimitiveTopology(RhiPrimitiveTopologyType type)
{
    m_CreationDesc.m_PrimitiveTopologyType = type;
    return RhiResult::Success;
}

RhiResult RhiPipelineState::SetDepthStencilState(const RhiDepthStencilDesc& desc)
{
    m_CreationDesc.m_DepthStencilDesc = desc;
    return RhiResult::Success;
}

RhiResult RhiPipelineState::SetDepthTargetFormat(RhiFormat dsvFormat)
{
    m_CreationDesc.m_DSVFormat = dsvFormat;
    return RhiResult::Success;
}

RhiResult RhiPipelineState::SetRenderTargetFormat(RhiFormat rtvFormat)
{
    return SetRenderTargetFormats(1, &rtvFormat);
}

RhiResult RhiPipelineState::SetRenderTargetFormats(uint32_t numRtv, const RhiFormat* rtvFormats)
{
    m_CreationDesc.m_NumRenderTargets = numRtv;
    for (int i = 0; i < numRtv; ++i)
        m_CreationDesc.m_RTVFormats[i] = rtvFormats[i];

    return RhiResult::Success;
}

RhiResult RhiPipelineState::SetSamplingDesc(uint32_t numMsaaSamples, uint32_t msaaQuality)
{
    m_CreationDesc.m_SampleDesc.m_NumMsaaSamples = numMsaaSamples;
    m_CreationDesc.m_SampleDesc.m_MsaaQuality = msaaQuality;
    return RhiResult::Success;
}

RhiResult RhiPipelineState::SetVertexShader(const void* binary, size_t size)
{
    m_CreationDesc.m_VS.m_Bytecode = binary;
    m_CreationDesc.m_VS.m_BytecodeLength = size;
    return RhiResult::Success;
}

RhiResult RhiPipelineState::SetPixelShader(const void* binary, size_t size)
{
    m_CreationDesc.m_PS.m_Bytecode = binary;
    m_CreationDesc.m_PS.m_BytecodeLength = size;
    return RhiResult::Success;
}

RhiResult RhiPipelineState::Finalize(RhiPipelineStateHandle& pipelineState)
{
    m_CreationDesc.m_RootSignature = m_RootSignature;
    return GraphicCore::GetDevice()->CreatePipelineState(m_CreationDesc, pipelineState);
}

ETH_NAMESPACE_END

