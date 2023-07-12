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

#include "graphics/pch.h"
#include "graphics/rhi/rhigraphicpipelinestate.h"
#include "graphics/rhi/dx12/dx12includes.h"
#include "graphics/rhi/dx12/dx12pipelinestate.h"

namespace Ether::Graphics
{
class Dx12GraphicPipelineStateDesc : public RhiGraphicPipelineStateDesc
{
public:
    Dx12GraphicPipelineStateDesc();
    ~Dx12GraphicPipelineStateDesc() override = default;

public:
    void SetBlendState(const RhiBlendDesc& desc) override;
    void SetRasterizerState(const RhiRasterizerDesc& desc) override;
    void SetInputLayout(const RhiInputElementDesc* descs, uint32_t numElements) override;
    void SetPrimitiveTopology(const RhiPrimitiveTopologyType& type) override;
    void SetDepthStencilState(const RhiDepthStencilDesc& desc) override;
    void SetDepthTargetFormat(RhiFormat dsvFormat) override;
    void SetRenderTargetFormat(RhiFormat rtvFormat) override;
    void SetRenderTargetFormats(const RhiFormat* rtvFormats, uint32_t numRtv) override;
    void SetRootSignature(const RhiRootSignature& rootSignature) override;
    void SetSamplingDesc(uint32_t numMsaaSamples, uint32_t msaaQuality) override;
    void SetVertexShader(const RhiShader& vs) override;
    void SetPixelShader(const RhiShader& ps) override;
    void SetNodeMask(uint32_t mask) override;
    void SetSampleMask(uint32_t mask) override;
    void Reset() override;

protected:
    friend class Dx12Device;
    std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputElements;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC m_Dx12PsoDesc;
};

class Dx12GraphicPipelineState : public Dx12PipelineState
{
public:
    Dx12GraphicPipelineState(const RhiGraphicPipelineStateDesc& desc) : Dx12PipelineState(desc) {}
    ~Dx12GraphicPipelineState() override = default;
};
} // namespace Ether::Graphics
