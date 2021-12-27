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

#pragma once

ETH_NAMESPACE_BEGIN

class RHIPipelineState
{
public:
    RHIPipelineState() = default;
    virtual ~RHIPipelineState() = default;

    inline void SetRootSignature(RHIRootSignatureHandle rootSignature) { m_RootSignature = rootSignature; }

public:
    RHIResult SetBlendState(const RHIBlendDesc& desc);
    RHIResult SetRasterizerState(const RHIRasterizerDesc& desc);
    RHIResult SetInputLayout(const RHIInputLayoutDesc& desc);
    RHIResult SetPrimitiveTopology(RHIPrimitiveTopologyType type);
    RHIResult SetDepthStencilState(const RHIDepthStencilDesc& desc);
    RHIResult SetDepthTargetFormat(RHIFormat dsvFormat);
    RHIResult SetRenderTargetFormat(RHIFormat rtvFormat);
    RHIResult SetRenderTargetFormats(uint32_t numRtv, const RHIFormat* rtvFormats);
    RHIResult SetSamplingDesc(uint32_t numMsaaSamples, uint32_t msaaQuality);
    RHIResult SetVertexShader(const void* binary, size_t size);
    RHIResult SetPixelShader(const void* binary, size_t size);
    RHIResult Finalize(RHIPipelineStateHandle& pipelineState);

protected:
    RHIRootSignatureHandle m_RootSignature;
    RHIPipelineStateDesc m_CreationDesc;
};

ETH_NAMESPACE_END

