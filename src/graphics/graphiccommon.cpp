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
    InitializeMaterials();
}

void Ether::Graphics::GraphicCommon::InitializeRasterizerStates()
{
    m_RasterizerDefault.m_FillMode = RhiFillMode::Solid;
    m_RasterizerDefault.m_CullMode = RhiCullMode::None;
    m_RasterizerDefault.m_FrontCounterClockwise = false;
    m_RasterizerDefault.m_DepthBias = 0;
    m_RasterizerDefault.m_DepthBiasClamp = 0;
    m_RasterizerDefault.m_SlopeScaledDepthBias = 0;
    m_RasterizerDefault.m_DepthClipEnable = true;
    m_RasterizerDefault.m_MultisampleEnable = false;
    m_RasterizerDefault.m_AntialiasedLineEnable = false;
    m_RasterizerDefault.m_ForcedSampleCount = 0;
    m_RasterizerDefaultCcw = m_RasterizerDefault;
    m_RasterizerDefaultCcw.m_FrontCounterClockwise = true;

    m_RasterizerWireframe = m_RasterizerDefault;
    m_RasterizerWireframe.m_FillMode = RhiFillMode::Wireframe;

    m_RasterizerWireframeCcw = m_RasterizerWireframeCcw;
    m_RasterizerWireframeCcw.m_FillMode = RhiFillMode::Wireframe;
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
    m_EmptyRootSignature = rootSignatureDesc->Compile("Empty Root Signature");
}

void Ether::Graphics::GraphicCommon::InitializeShaders()
{
}

void Ether::Graphics::GraphicCommon::InitializePipelineStates()
{
}

void Ether::Graphics::GraphicCommon::InitializeSamplers()
{
    m_PointSampler_Clamp = {};
    m_PointSampler_Clamp.m_Filter = RhiFilter::MinMagMipPoint;
    m_PointSampler_Clamp.m_AddressU = RhiTextureAddressMode::Clamp;
    m_PointSampler_Clamp.m_AddressV = RhiTextureAddressMode::Clamp;
    m_PointSampler_Clamp.m_AddressW = RhiTextureAddressMode::Clamp;
    m_PointSampler_Clamp.m_MipLodBias = 0;
    m_PointSampler_Clamp.m_MaxAnisotropy = 0;
    m_PointSampler_Clamp.m_ComparisonFunc = RhiComparator::Never;
    m_PointSampler_Clamp.m_MinLod = 0;
    m_PointSampler_Clamp.m_MaxLod = std::numeric_limits<float_t>().max();

    m_PointSampler_Wrap = m_PointSampler_Clamp;
    m_PointSampler_Wrap.m_AddressU = RhiTextureAddressMode::Wrap;
    m_PointSampler_Wrap.m_AddressV = RhiTextureAddressMode::Wrap;
    m_PointSampler_Wrap.m_AddressW = RhiTextureAddressMode::Wrap;

    m_PointSampler_Border = m_PointSampler_Clamp;
    m_PointSampler_Border.m_AddressU = RhiTextureAddressMode::Border;
    m_PointSampler_Border.m_AddressV = RhiTextureAddressMode::Border;
    m_PointSampler_Border.m_AddressW = RhiTextureAddressMode::Border;
    m_PointSampler_Border.m_BorderColor = RhiBorderColor::OpaqueBlack;

    m_LinearSampler_Clamp = m_PointSampler_Clamp;
    m_LinearSampler_Clamp.m_Filter = RhiFilter::Anisotropic;
    m_LinearSampler_Wrap = m_PointSampler_Wrap;
    m_LinearSampler_Wrap.m_Filter = RhiFilter::Anisotropic;
    m_LinearSampler_Border = m_PointSampler_Border;
    m_LinearSampler_Border.m_Filter = RhiFilter::Anisotropic;

    m_SamplerIndex_Point_Clamp = GraphicCore::GetBindlessDescriptorManager().RegisterSampler("Sampler_PointClamp", m_PointSampler_Clamp);
    m_SamplerIndex_Point_Wrap = GraphicCore::GetBindlessDescriptorManager().RegisterSampler("Sampler_PointWrap", m_PointSampler_Wrap);
    m_SamplerIndex_Point_Border =  GraphicCore::GetBindlessDescriptorManager().RegisterSampler("Sampler_PointBorder", m_PointSampler_Border);
    m_SamplerIndex_Linear_Clamp = GraphicCore::GetBindlessDescriptorManager().RegisterSampler("Sampler_LinearClamp", m_LinearSampler_Clamp);
    m_SamplerIndex_Linear_Wrap = GraphicCore::GetBindlessDescriptorManager().RegisterSampler("Sampler_LinearWrap", m_LinearSampler_Wrap);
    m_SamplerIndex_Linear_Border = GraphicCore::GetBindlessDescriptorManager().RegisterSampler("Sampler_LinearBorder", m_LinearSampler_Border);
}

void Ether::Graphics::GraphicCommon::InitializeDefaultTextures()
{
}

void Ether::Graphics::GraphicCommon::InitializeMaterials()
{
    m_DefaultMaterial = std::make_unique<Material>();
    m_DefaultMaterial->SetBaseColor({ 1, 1, 1, 1 });
    m_DefaultMaterial->SetSpecularColor({ 0.5, 0.5, 0.5, 0.5 });

    m_ErrorMaterial = std::make_unique<Material>();
    m_ErrorMaterial->SetBaseColor({ 1, 0, 1, 1 });
    m_ErrorMaterial->SetSpecularColor({ 0, 0, 0, 0 });
}
