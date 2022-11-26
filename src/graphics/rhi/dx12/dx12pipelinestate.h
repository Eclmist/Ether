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
#include "graphics/rhi/rhipipelinestate.h"
#include "graphics/rhi/rhishader.h"
#include "graphics/rhi/dx12/dx12includes.h"

namespace Ether::Graphics
{
    class Dx12PipelineStateDesc : public RhiPipelineStateDesc
    {
    public:
        Dx12PipelineStateDesc();
        ~Dx12PipelineStateDesc() override = default;

    public:
        void SetBlendState(RhiBlendDesc desc) override;
        void SetRasterizerState(RhiRasterizerDesc desc) override;
        void SetInputLayout(RhiInputLayoutDesc desc) override;
        void SetPrimitiveTopology(RhiPrimitiveTopologyType type) override;
        void SetDepthStencilState(RhiDepthStencilDesc desc) override;
        void SetDepthTargetFormat(RhiFormat dsvFormat) override;
        void SetRenderTargetFormat(RhiFormat rtvFormat) override;
        void SetRenderTargetFormats(uint32_t numRtv, const RhiFormat* rtvFormats) override;
        void SetRootSignature(const RhiRootSignature& rootSignature) override;
        void SetSamplingDesc(uint32_t numMsaaSamples, uint32_t msaaQuality) override;
        void SetVertexShader(RhiShader& vs) override;
        void SetPixelShader(RhiShader& ps) override;
        void SetNodeMask(uint32_t mask) override;
        void SetSampleMask(uint32_t mask) override;

    public:
        void Reset() override;

    protected:
        friend class Dx12Device;
        std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputElements;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC m_Dx12PsoDesc;
    };

    class Dx12PipelineState : public RhiPipelineState
    {
    public:
        Dx12PipelineState(const RhiPipelineStateDesc& desc) : RhiPipelineState(desc) {}
        ~Dx12PipelineState() override = default;

    private:
        friend class Dx12Device;
        friend class Dx12CommandList;
        wrl::ComPtr<ID3D12PipelineState> m_PipelineState;
    };
}

