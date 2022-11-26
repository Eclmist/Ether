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
#include "graphics/rhi/rhirootsignature.h"

namespace Ether::Graphics
{
    class RhiPipelineStateDesc
    {
    public:
        RhiPipelineStateDesc() = default;
        virtual ~RhiPipelineStateDesc() {}

    public:
        virtual void SetBlendState(RhiBlendDesc desc) = 0;
        virtual void SetRasterizerState(RhiRasterizerDesc desc) = 0;
        virtual void SetInputLayout(RhiInputLayoutDesc desc) = 0;
        virtual void SetPrimitiveTopology(RhiPrimitiveTopologyType type) = 0;
        virtual void SetDepthStencilState(RhiDepthStencilDesc desc) = 0;
        virtual void SetDepthTargetFormat(RhiFormat dsvFormat) = 0;
        virtual void SetRenderTargetFormat(RhiFormat rtvFormat) = 0;
        virtual void SetRenderTargetFormats(uint32_t numRtv, const RhiFormat* rtvFormats) = 0;
        virtual void SetRootSignature(const RhiRootSignature& rootSignature) = 0;
        virtual void SetSamplingDesc(uint32_t numMsaaSamples, uint32_t msaaQuality) = 0;
        virtual void SetVertexShader(RhiShader& vs) = 0;
        virtual void SetPixelShader(RhiShader& ps) = 0;
        virtual void SetNodeMask(uint32_t mask) = 0;
        virtual void SetSampleMask(uint32_t mask) = 0;
        virtual void Reset() = 0;

    public:
        std::unique_ptr<RhiPipelineState> Compile() const;
        bool RequiresShaderCompilation() const;
        void CompileShaders();

    protected:
        std::unordered_map<RhiShaderType, RhiShader*> m_Shaders;
    };

    class RhiPipelineState
    {
    public:
        RhiPipelineState(const RhiPipelineStateDesc& desc) : m_CreationDesc(desc) {}
        virtual ~RhiPipelineState() = default;

    protected:
        const RhiPipelineStateDesc& m_CreationDesc;
    };
}

