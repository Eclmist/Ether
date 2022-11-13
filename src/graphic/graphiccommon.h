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
    void InitializeSamplers();
    void InitializeDefaultTextures();

public:
    // TODO: Privatize with getters
    RhiRasterizerDesc m_RasterizerDefault;
    RhiRasterizerDesc m_RasterizerDefaultCw;
    RhiRasterizerDesc m_RasterizerWireframe;
    RhiRasterizerDesc m_RasterizerWireframeCw;

    RhiBlendDesc m_BlendDisabled;
    RhiBlendDesc m_BlendPreMultiplied;
    RhiBlendDesc m_BlendTraditional;
    RhiBlendDesc m_BlendAdditive;
    RhiBlendDesc m_BlendTraditionalAdditive;

    RhiDepthStencilDesc m_DepthStateDisabled;
    RhiDepthStencilDesc m_DepthStateReadWrite;
    RhiDepthStencilDesc m_DepthStateReadOnly;
    RhiDepthStencilDesc m_DepthStateTestEqual;

    RhiInputLayoutDesc m_DefaultInputLayout;

    std::unique_ptr<class Shader> m_DefaultVS;
    std::unique_ptr<class Shader> m_DefaultPS;

    RhiPipelineStateHandle m_DefaultPSO;
    RhiPipelineStateHandle m_DefaultWireframePSO;
    RhiRootSignatureHandle m_BindlessRootSignature;
    RhiRootSignatureFlag m_BindlessRootSignatureFlags;

    RhiSamplerParameterDesc m_PointSampler;
    RhiSamplerParameterDesc m_BilinearSampler;
    RhiSamplerParameterDesc m_EnvMapSampler;

    CompiledTexture m_ErrorTexture2D;
};

ETH_NAMESPACE_END

