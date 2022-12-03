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

#include "graphics/graphiccore.h"
#include "graphics/graphiccommon.h"

Ether::Graphics::GraphicCommon::GraphicCommon()
{
    InitializeRasterizerStates();
    InitializeDepthStates();
    InitializeBlendingStates();
    InitializeSamplers();
    InitializeRootSignatures();
    InitializeShaders();
    InitializePipelineStates();
    InitializeDefaultTextures();
}

void Ether::Graphics::GraphicCommon::InitializeRasterizerStates()
{
    m_RasterizerDefault.m_FillMode = RhiFillMode::Solid;
    m_RasterizerDefault.m_CullMode = RhiCullMode::Back;
    m_RasterizerDefault.m_FrontCounterClockwise = true;
    m_RasterizerDefault.m_DepthBias = 0;
    m_RasterizerDefault.m_DepthBiasClamp = 0;
    m_RasterizerDefault.m_SlopeScaledDepthBias = 0;
    m_RasterizerDefault.m_DepthClipEnable = true;
    m_RasterizerDefault.m_MultisampleEnable = false;
    m_RasterizerDefault.m_AntialiasedLineEnable = false;
    m_RasterizerDefault.m_ForcedSampleCount = 0;
    m_RasterizerDefaultCw = m_RasterizerDefault;
    m_RasterizerDefaultCw.m_FrontCounterClockwise = false;

    m_RasterizerWireframe = m_RasterizerDefault;
    m_RasterizerWireframe.m_FillMode = RhiFillMode::Wireframe;

    m_RasterizerWireframeCw = m_RasterizerWireframe;
    m_RasterizerWireframeCw.m_FrontCounterClockwise = false;
}

void Ether::Graphics::GraphicCommon::InitializeDepthStates()
{
    m_DepthStateDisabled.m_DepthEnabled = false;
    m_DepthStateDisabled.m_DepthWriteMask = RhiDepthWriteMask::Zero;
    m_DepthStateDisabled.m_DepthComparator = RhiComparator::Always;
    m_DepthStateDisabled.m_StencilEnabled = false;
    m_DepthStateDisabled.m_StencilReadMask = 0xff;
    m_DepthStateDisabled.m_StencilWriteMask = 0xff;
    m_DepthStateDisabled.m_FrontFace.m_StencilFunc = RhiComparator::Always;
    m_DepthStateDisabled.m_FrontFace.m_StencilPassOp = RhiDepthStencilOperation::Keep;
    m_DepthStateDisabled.m_FrontFace.m_StencilFailOp = RhiDepthStencilOperation::Keep;
    m_DepthStateDisabled.m_FrontFace.m_StencilDepthFailOp = RhiDepthStencilOperation::Keep;
    m_DepthStateDisabled.m_BackFace = m_DepthStateDisabled.m_FrontFace;

    m_DepthStateReadWrite = m_DepthStateDisabled;
    m_DepthStateReadWrite.m_DepthEnabled = true;
    m_DepthStateReadWrite.m_DepthWriteMask = RhiDepthWriteMask::All;
    m_DepthStateReadWrite.m_DepthComparator = RhiComparator::LessEqual;

    m_DepthStateReadOnly = m_DepthStateReadWrite;
    m_DepthStateReadOnly.m_DepthWriteMask = RhiDepthWriteMask::Zero;

    m_DepthStateTestEqual = m_DepthStateReadOnly;
    m_DepthStateTestEqual.m_DepthComparator = RhiComparator::Equal;
}

void Ether::Graphics::GraphicCommon::InitializeBlendingStates()
{
    RhiBlendDesc alphaBlend = {};
    alphaBlend.m_BlendingEnabled = false;
    alphaBlend.m_SrcBlend = RhiBlendType::SrcAlpha;
    alphaBlend.m_DestBlend = RhiBlendType::InvSrcAlpha;
    alphaBlend.m_BlendOp = RhiBlendOperation::Add;
    alphaBlend.m_SrcBlendAlpha = RhiBlendType::One;
    alphaBlend.m_DestBlendAlpha = RhiBlendType::InvSrcAlpha;
    alphaBlend.m_BlendOpAlpha = RhiBlendOperation::Add;
    alphaBlend.m_WriteMask = RhiRenderTargetWriteMask::All;

    m_BlendDisabled = alphaBlend;

    alphaBlend.m_BlendingEnabled = true;
    m_BlendTraditional = alphaBlend;

    alphaBlend.m_SrcBlend = RhiBlendType::One;
    m_BlendPreMultiplied = alphaBlend;

    alphaBlend.m_DestBlend = RhiBlendType::One;
    m_BlendAdditive = alphaBlend;

    alphaBlend.m_SrcBlend = RhiBlendType::SrcAlpha;
    m_BlendTraditionalAdditive = alphaBlend;
}

void Ether::Graphics::GraphicCommon::InitializeRootSignatures()
{
    std::unique_ptr<RhiRootSignatureDesc> rootSignatureDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(0, 0);
    rootSignatureDesc->SetFlags(RhiRootSignatureFlag::None);
    m_EmptyRootSignature = rootSignatureDesc->Compile();

    rootSignatureDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(0, 3);
    rootSignatureDesc->SetFlags(RhiRootSignatureFlag::None);
    rootSignatureDesc->SetAsSampler(0, m_PointSampler, RhiShaderVisibility::All);
    rootSignatureDesc->SetAsSampler(1, m_BilinearSampler, RhiShaderVisibility::All);
    rootSignatureDesc->SetAsSampler(2, m_EnvMapSampler, RhiShaderVisibility::All);
    m_DefaultRootSignature = rootSignatureDesc->Compile();
}

void Ether::Graphics::GraphicCommon::InitializeShaders()
{

}

void Ether::Graphics::GraphicCommon::InitializePipelineStates()
{

}

void Ether::Graphics::GraphicCommon::InitializeSamplers()
{
    m_PointSampler = {};
    m_PointSampler.m_Filter = RhiFilter::MinMagMipPoint;
    m_PointSampler.m_AddressU = RhiTextureAddressMode::Wrap;
    m_PointSampler.m_AddressV = RhiTextureAddressMode::Wrap;
    m_PointSampler.m_AddressW = RhiTextureAddressMode::Wrap;
    m_PointSampler.m_MipLodBias = 0;
    m_PointSampler.m_MaxAnisotropy = 0;
    m_PointSampler.m_ComparisonFunc = RhiComparator::Never;
    m_PointSampler.m_MinLod = 0;
    m_PointSampler.m_MinLod = std::numeric_limits<float_t>().max();

    m_BilinearSampler = m_PointSampler;
    m_BilinearSampler.m_Filter = RhiFilter::MinMagMipLinear;

    m_EnvMapSampler = m_BilinearSampler;
}

void Ether::Graphics::GraphicCommon::InitializeDefaultTextures()
{

}
