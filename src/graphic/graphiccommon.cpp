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

#include "graphiccommon.h"
#include "graphic/rhi/rhidevice.h"
#include "graphic/rhi/rhipipelinestate.h"
#include "graphic/rhi/rhirootparameter.h"
#include "graphic/rhi/rhirootsignature.h"

ETH_NAMESPACE_BEGIN

GraphicCommon::GraphicCommon()
    : m_RasterizerDefault()
    , m_RasterizerDefaultCw()
    , m_RasterizerWireframe()
    , m_RasterizerWireframeCw()
    , m_BlendDisabled()
    , m_BlendPreMultiplied()
    , m_BlendTraditional()
    , m_BlendAdditive()
    , m_BlendTraditionalAdditive()
    , m_DepthStateDisabled()
    , m_DepthStateReadWrite()
    , m_DepthStateReadOnly()
    , m_DepthStateTestEqual()
    , m_DefaultInputLayout()
{
    InitializeRasterizerStates();
    InitializeDepthStates();
    InitializeBlendingStates();
    InitializeRootSignatures();
    InitializeShaders();
    InitializePipelineStates();
    InitializeSamplers();
}

GraphicCommon::~GraphicCommon()
{
    m_DefaultPSO.Destroy();
    m_DefaultWireframePSO.Destroy();
    m_EmptyRootSignature.Destroy();
}

void GraphicCommon::InitializeRasterizerStates()
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

void GraphicCommon::InitializeDepthStates()
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

void GraphicCommon::InitializeBlendingStates()
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

void GraphicCommon::InitializeRootSignatures()
{
    m_EmptyRootSignature.SetName(L"GraphicCommon::EmptyRootSignature");

    m_DefaultRootSignatureFlags = 
        RhiRootSignatureFlag::AllowIAInputLayout |
        RhiRootSignatureFlag::DenyHSRootAccess |
        RhiRootSignatureFlag::DenyGSRootAccess |
        RhiRootSignatureFlag::DenyDSRootAccess;

    RhiRootSignature tempRS(2, 0);
    tempRS[0]->SetAsConstantBufferView({ 0, 0, RhiShaderVisibility::All });
    tempRS[1]->SetAsConstant({ 1, 0, RhiShaderVisibility::All, 32 });

    tempRS.Finalize(m_DefaultRootSignatureFlags, m_EmptyRootSignature);
}

RhiInputElementDesc inputElementDesc[4] = 
{
    { "POSITION", 0, RhiFormat::R32G32B32Float, 0, D3D12_APPEND_ALIGNED_ELEMENT, RhiInputClassification::PerVertexData, 0 },
    { "NORMAL", 0, RhiFormat::R32G32B32Float, 0, D3D12_APPEND_ALIGNED_ELEMENT, RhiInputClassification::PerVertexData, 0 },
    { "TANGENT", 0, RhiFormat::R32G32B32A32Float, 0, D3D12_APPEND_ALIGNED_ELEMENT, RhiInputClassification::PerVertexData, 0 },
    { "TEXCOORD", 0, RhiFormat::R32G32Float, 0, D3D12_APPEND_ALIGNED_ELEMENT, RhiInputClassification::PerVertexData, 0 },
};

void GraphicCommon::InitializeShaders()
{
    m_DefaultInputLayout.m_NumElements = 4;
    m_DefaultInputLayout.m_InputElementDescs = inputElementDesc;

    m_DefaultVS = std::make_unique<Shader>(L"default.hlsl", L"VS_Main", L"vs_6_0", ShaderType::Vertex);
    m_DefaultPS = std::make_unique<Shader>(L"default.hlsl", L"PS_Main", L"ps_6_0", ShaderType::Pixel);

    m_DefaultVS->Compile();
    m_DefaultPS->Compile();
    m_DefaultVS->SetRecompiled(false);
    m_DefaultPS->SetRecompiled(false);
}

void GraphicCommon::InitializePipelineStates()
{
    m_DefaultPSO.SetName(L"GraphicCommon::DefaultPSO");
    m_DefaultWireframePSO.SetName(L"GraphicCommon::DefaultWireframePSO");

    RhiPipelineState creationPSO;
    creationPSO.SetBlendState(m_BlendDisabled);
    creationPSO.SetRasterizerState(m_RasterizerDefault);
    creationPSO.SetPrimitiveTopology(RhiPrimitiveTopologyType::Triangle);
    creationPSO.SetVertexShader(m_DefaultVS->GetCompiledShader(), m_DefaultVS->GetCompiledShaderSize());
    creationPSO.SetPixelShader(m_DefaultPS->GetCompiledShader(), m_DefaultPS->GetCompiledShaderSize());
    creationPSO.SetInputLayout(m_DefaultInputLayout);
    creationPSO.SetRenderTargetFormat(RhiFormat::R8G8B8A8Unorm);
    creationPSO.SetDepthTargetFormat(RhiFormat::D24UnormS8Uint);
    creationPSO.SetDepthStencilState(m_DepthStateReadWrite);
    creationPSO.SetSamplingDesc(1, 0);
    creationPSO.SetRootSignature(m_EmptyRootSignature);
    creationPSO.Finalize(m_DefaultPSO);

    creationPSO.SetRasterizerState(m_RasterizerWireframe);
    creationPSO.Finalize(m_DefaultWireframePSO);
}

void GraphicCommon::InitializeSamplers()
{
    m_PointSampler.m_Filter = RhiFilter::MinMagMipPoint;
    m_PointSampler.m_AddressU = RhiTextureAddressMode::Wrap;
    m_PointSampler.m_AddressV = RhiTextureAddressMode::Wrap;
    m_PointSampler.m_AddressW = RhiTextureAddressMode::Wrap;
    m_PointSampler.m_MipLODBias = 0;
    m_PointSampler.m_MaxAnisotropy = 0;
    m_PointSampler.m_ComparisonFunc = RhiComparator::Never;
    m_PointSampler.m_MinLOD = 0;
    m_PointSampler.m_MinLOD = std::numeric_limits<float_t>().max();
    m_PointSampler.m_ShaderRegister = 0;
    m_PointSampler.m_RegisterSpace = 0;
    m_PointSampler.m_ShaderVisibility = RhiShaderVisibility::All;

    m_BilinearSampler = m_PointSampler;
    m_BilinearSampler.m_Filter = RhiFilter::MinMagMipLinear;
    m_BilinearSampler.m_ShaderRegister = 1;

    m_EnvMapSampler = m_BilinearSampler;
    m_EnvMapSampler.m_ShaderRegister = 2;

}

ETH_NAMESPACE_END

