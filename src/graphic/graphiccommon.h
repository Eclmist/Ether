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

class GraphicCommon : public NonCopyable
{
public:
    GraphicCommon();
    ~GraphicCommon();

public:
    void InitializeRasterizerStates();
    void InitializeDepthStates();
    void InitializeBlendingStates();
    void InitializeRootSignatures();
    void InitializeShaders();
    void InitializePipelineStates();

public:
    D3D12_RASTERIZER_DESC m_RasterizerDefault;
    D3D12_RASTERIZER_DESC m_RasterizerDefaultCw;
    D3D12_RASTERIZER_DESC m_RasterizerWireframe;
    D3D12_RASTERIZER_DESC m_RasterizerWireframeCw;

    D3D12_BLEND_DESC m_BlendDisabled;
    D3D12_BLEND_DESC m_BlendPreMultiplied;
    D3D12_BLEND_DESC m_BlendTraditional;
    D3D12_BLEND_DESC m_BlendAdditive;
    D3D12_BLEND_DESC m_BlendTraditionalAdditive;

    D3D12_DEPTH_STENCIL_DESC m_DepthStateDisabled;
    D3D12_DEPTH_STENCIL_DESC m_DepthStateReadWrite;
    D3D12_DEPTH_STENCIL_DESC m_DepthStateReadOnly;
    D3D12_DEPTH_STENCIL_DESC m_DepthStateTestEqual;

    D3D12_INPUT_LAYOUT_DESC m_DefaultInputLayout;

    std::unique_ptr<Shader> m_DefaultVS;
    std::unique_ptr<Shader> m_DefaultPS;
    std::unique_ptr<GraphicPipelineState> m_DefaultPSO;
    std::unique_ptr<GraphicPipelineState> m_DefaultWireframePSO;
    std::unique_ptr<RootSignature> m_DefaultRootSignature;
};

ETH_NAMESPACE_END

