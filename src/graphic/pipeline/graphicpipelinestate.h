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

#include "pipelinestate.h"
#include "graphic/pipeline/rootsignature.h"

ETH_NAMESPACE_BEGIN

class GraphicPipelineState : public PipelineState
{
public:
    GraphicPipelineState(const std::wstring& name = L"Unnamed Graphic PSO");
    ~GraphicPipelineState() = default;
    
public:
    void SetNumLayoutElements(uint32_t numLayoutElements);
    void SetInputLayout(const D3D12_INPUT_ELEMENT_DESC* desc);
    void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE type);
    void SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& desc);
    void SetDepthTargetFormat(DXGI_FORMAT dsvFormat);
    void SetRenderTargetFormat(DXGI_FORMAT rtvFormat);
    void SetRenderTargetFormats(uint32_t numRtv, const DXGI_FORMAT* rtvFormats);
    void SetSamplingDesc(uint32_t numMsaaSamples, uint32_t msaaQuality);
    void SetVertexShader(const void* binary, size_t size);
    void SetPixelShader(const void* binary, size_t size);

    void Finalize(const std::wstring& name);

private:
    D3D12_GRAPHICS_PIPELINE_STATE_DESC m_Desc;
};

ETH_NAMESPACE_END

