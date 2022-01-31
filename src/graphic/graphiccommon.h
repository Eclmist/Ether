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

public:
    // TODO: Privatize with getters
    RHIRasterizerDesc m_RasterizerDefault;
    RHIRasterizerDesc m_RasterizerDefaultCw;
    RHIRasterizerDesc m_RasterizerWireframe;
    RHIRasterizerDesc m_RasterizerWireframeCw;

    RHIBlendDesc m_BlendDisabled;
    RHIBlendDesc m_BlendPreMultiplied;
    RHIBlendDesc m_BlendTraditional;
    RHIBlendDesc m_BlendAdditive;
    RHIBlendDesc m_BlendTraditionalAdditive;

    RHIDepthStencilDesc m_DepthStateDisabled;
    RHIDepthStencilDesc m_DepthStateReadWrite;
    RHIDepthStencilDesc m_DepthStateReadOnly;
    RHIDepthStencilDesc m_DepthStateTestEqual;

    RHIInputLayoutDesc m_DefaultInputLayout;

    std::unique_ptr<class Shader> m_DefaultVS;
    std::unique_ptr<class Shader> m_DefaultPS;

    RHIPipelineStateHandle m_DefaultPSO;
    RHIPipelineStateHandle m_DefaultWireframePSO;
    RHIRootSignatureHandle m_EmptyRootSignature;
    RHIRootSignatureFlags m_DefaultRootSignatureFlags;

    RHISamplerParameterDesc m_PointSampler;
    RHISamplerParameterDesc m_BilinearSampler;
};

ETH_NAMESPACE_END

