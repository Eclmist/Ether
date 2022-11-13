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

#pragma once

ETH_NAMESPACE_BEGIN

class RhiPipelineState
{
public:
    RhiPipelineState() = default;
    virtual ~RhiPipelineState() = default;

    inline void SetRootSignature(RhiRootSignatureHandle rootSignature) { m_RootSignature = rootSignature; }

public:
    RhiResult SetBlendState(const RhiBlendDesc& desc);
    RhiResult SetRasterizerState(const RhiRasterizerDesc& desc);
    RhiResult SetInputLayout(const RhiInputLayoutDesc& desc);
    RhiResult SetPrimitiveTopology(RhiPrimitiveTopologyType type);
    RhiResult SetDepthStencilState(const RhiDepthStencilDesc& desc);
    RhiResult SetDepthTargetFormat(RhiFormat dsvFormat);
    RhiResult SetRenderTargetFormat(RhiFormat rtvFormat);
    RhiResult SetRenderTargetFormats(uint32_t numRtv, const RhiFormat* rtvFormats);
    RhiResult SetSamplingDesc(uint32_t numMsaaSamples, uint32_t msaaQuality);
    RhiResult SetVertexShader(const void* binary, size_t size);
    RhiResult SetPixelShader(const void* binary, size_t size);
    RhiResult Finalize(RhiPipelineStateHandle& pipelineState);

protected:
    RhiRootSignatureHandle m_RootSignature;
    RhiPipelineStateDesc m_CreationDesc = {};
};

ETH_NAMESPACE_END

