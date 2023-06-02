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
#include "graphics/memory/descriptorallocator.h"

namespace Ether::Graphics
{
class GraphicCommon : public NonCopyable, public NonMovable
{
public:
    GraphicCommon();
    ~GraphicCommon() = default;

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

    RhiSamplerParameterDesc m_PointSampler;
    RhiSamplerParameterDesc m_BilinearSampler;
    RhiSamplerParameterDesc m_EnvMapSampler;

    std::unique_ptr<RhiRootSignature> m_EmptyRootSignature;
    std::unique_ptr<RhiRootSignature> m_DefaultRootSignature;
};
} // namespace Ether::Graphics
