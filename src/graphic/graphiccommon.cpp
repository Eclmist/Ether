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
}

GraphicCommon::~GraphicCommon()
{
    m_DefaultPSO.Destroy();
    m_DefaultWireframePSO.Destroy();
    m_DefaultRootSignature.Destroy();
}

void GraphicCommon::InitializeRasterizerStates()
{
    m_RasterizerDefault.m_FillMode = RHIFillMode::Solid;
    m_RasterizerDefault.m_CullMode = RHICullMode::Back;
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
    m_RasterizerWireframe.m_FillMode = RHIFillMode::Wireframe;

    m_RasterizerWireframeCw = m_RasterizerWireframe;
    m_RasterizerWireframeCw.m_FrontCounterClockwise = false;
}

void GraphicCommon::InitializeDepthStates()
{
    m_DepthStateDisabled.m_DepthEnabled = false;
    m_DepthStateDisabled.m_DepthWriteMask = RHIDepthWriteMask::Zero;
    m_DepthStateDisabled.m_DepthComparator = RHIComparator::Always;
    m_DepthStateDisabled.m_StencilEnabled = false;
    m_DepthStateDisabled.m_StencilReadMask = 0xff;
    m_DepthStateDisabled.m_StencilWriteMask = 0xff;
    m_DepthStateDisabled.m_FrontFace.m_StencilFunc = RHIComparator::Always;
    m_DepthStateDisabled.m_FrontFace.m_StencilPassOp = RHIDepthStencilOperation::Keep;
    m_DepthStateDisabled.m_FrontFace.m_StencilFailOp = RHIDepthStencilOperation::Keep;
    m_DepthStateDisabled.m_FrontFace.m_StencilDepthFailOp = RHIDepthStencilOperation::Keep;
    m_DepthStateDisabled.m_BackFace = m_DepthStateDisabled.m_FrontFace;

    m_DepthStateReadWrite = m_DepthStateDisabled;
    m_DepthStateReadWrite.m_DepthEnabled = true;
    m_DepthStateReadWrite.m_DepthWriteMask = RHIDepthWriteMask::All;
    m_DepthStateReadWrite.m_DepthComparator = RHIComparator::LessEqual;

    m_DepthStateReadOnly = m_DepthStateReadWrite;
    m_DepthStateReadOnly.m_DepthWriteMask = RHIDepthWriteMask::Zero;

    m_DepthStateTestEqual = m_DepthStateReadOnly;
    m_DepthStateTestEqual.m_DepthComparator = RHIComparator::Equal;
}

void GraphicCommon::InitializeBlendingStates()
{
    RHIBlendDesc alphaBlend = {};
    alphaBlend.m_BlendingEnabled = false;
    alphaBlend.m_SrcBlend = RHIBlendType::SrcAlpha;
    alphaBlend.m_DestBlend = RHIBlendType::InvSrcAlpha;
    alphaBlend.m_BlendOp = RHIBlendOperation::Add;
    alphaBlend.m_SrcBlendAlpha = RHIBlendType::One;
    alphaBlend.m_DestBlendAlpha = RHIBlendType::InvSrcAlpha;
    alphaBlend.m_BlendOpAlpha = RHIBlendOperation::Add;
    alphaBlend.m_WriteMask = static_cast<RHIColorChannels>(RHIColorChannel::All);

    m_BlendDisabled = alphaBlend;

    alphaBlend.m_BlendingEnabled = true;
    m_BlendTraditional = alphaBlend;

    alphaBlend.m_SrcBlend = RHIBlendType::One;
    m_BlendPreMultiplied = alphaBlend;

    alphaBlend.m_DestBlend = RHIBlendType::One;
    m_BlendAdditive = alphaBlend;

    alphaBlend.m_SrcBlend = RHIBlendType::SrcAlpha;
    m_BlendTraditionalAdditive = alphaBlend;
}

void GraphicCommon::InitializeRootSignatures()
{
    m_DefaultRootSignature.SetName(L"GraphicCommon::DefaultRootSignature");

    RHIRootSignatureFlags rootSignatureFlags =
        static_cast<RHIRootSignatureFlags>(RHIRootSignatureFlag::AllowIAInputLayout) |
        static_cast<RHIRootSignatureFlags>(RHIRootSignatureFlag::DenyHSRootAccess) |
        static_cast<RHIRootSignatureFlags>(RHIRootSignatureFlag::DenyGSRootAccess) |
        static_cast<RHIRootSignatureFlags>(RHIRootSignatureFlag::DenyDSRootAccess);

    RHIRootSignature tempRS(2, 0);

    tempRS[0]->SetAsConstantBufferView({ 0, 0, RHIShaderVisibility::All });
    tempRS[1]->SetAsConstant({ 1, 0, RHIShaderVisibility::All, 32 });
    tempRS.Finalize(rootSignatureFlags, m_DefaultRootSignature);
}

RHIInputElementDesc inputElementDesc[4] = 
{
    { "POSITION", 0, RHIFormat::R32G32B32Float, 0, D3D12_APPEND_ALIGNED_ELEMENT, RHIInputClassification::PerVertexData, 0 },
    { "NORMAL", 0, RHIFormat::R32G32B32Float, 0, D3D12_APPEND_ALIGNED_ELEMENT, RHIInputClassification::PerVertexData, 0 },
    { "TANGENT", 0, RHIFormat::R32G32B32A32Float, 0, D3D12_APPEND_ALIGNED_ELEMENT, RHIInputClassification::PerVertexData, 0 },
    { "TEXCOORD", 0, RHIFormat::R32G32Float, 0, D3D12_APPEND_ALIGNED_ELEMENT, RHIInputClassification::PerVertexData, 0 },
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

    RHIPipelineState creationPSO;
    creationPSO.SetBlendState(m_BlendDisabled);
    creationPSO.SetRasterizerState(m_RasterizerDefault);
    creationPSO.SetPrimitiveTopology(RHIPrimitiveTopologyType::Triangle);
    creationPSO.SetVertexShader(m_DefaultVS->GetCompiledShader(), m_DefaultVS->GetCompiledShaderSize());
    creationPSO.SetPixelShader(m_DefaultPS->GetCompiledShader(), m_DefaultPS->GetCompiledShaderSize());
    creationPSO.SetInputLayout(m_DefaultInputLayout);
    creationPSO.SetRenderTargetFormat(RHIFormat::R8G8B8A8Unorm);
    creationPSO.SetDepthTargetFormat(RHIFormat::D24UnormS8Uint);
    creationPSO.SetDepthStencilState(m_DepthStateReadWrite);
    creationPSO.SetSamplingDesc(1, 0);
    creationPSO.SetRootSignature(m_DefaultRootSignature);
    creationPSO.Finalize(m_DefaultPSO);

    creationPSO.SetRasterizerState(m_RasterizerWireframe);
    creationPSO.Finalize(m_DefaultWireframePSO);
}

ETH_NAMESPACE_END

