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

namespace Ether::Graphics
{
class RhiGraphicPipelineStateDesc : public RhiPipelineStateDesc
{
public:
    RhiGraphicPipelineStateDesc() = default;
    virtual ~RhiGraphicPipelineStateDesc() {}

public:
    virtual void SetVertexShader(const RhiShader& vs) = 0;
    virtual void SetPixelShader(const RhiShader& ps) = 0;
    virtual void SetBlendState(const RhiBlendDesc& desc) = 0;
    virtual void SetRasterizerState(const RhiRasterizerDesc& desc) = 0;
    virtual void SetInputLayout(const RhiInputElementDesc* descs, uint32_t numElements) = 0;
    virtual void SetPrimitiveTopology(const RhiPrimitiveTopologyType& type) = 0;
    virtual void SetDepthStencilState(const RhiDepthStencilDesc& desc) = 0;
    virtual void SetDepthTargetFormat(RhiFormat dsvFormat) = 0;
    virtual void SetRenderTargetFormat(RhiFormat rtvFormat) = 0;
    virtual void SetRenderTargetFormats(const RhiFormat* rtvFormats, uint32_t numRtv) = 0;
    virtual void SetSamplingDesc(uint32_t numMsaaSamples, uint32_t msaaQuality) = 0;
    virtual void SetSampleMask(uint32_t mask) = 0;

public:
    virtual std::unique_ptr<RhiPipelineState> Compile(const char* name) const override;
};
} // namespace Ether::Graphics
