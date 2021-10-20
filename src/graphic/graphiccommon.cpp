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

#include "graphiccommon.h"

ETH_NAMESPACE_BEGIN

GraphicCommon::GraphicCommon()
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
}

void GraphicCommon::InitializeRasterizerStates()
{
    m_RasterizerDefault.FillMode = D3D12_FILL_MODE_SOLID;
    m_RasterizerDefault.CullMode = D3D12_CULL_MODE_BACK;
    m_RasterizerDefault.FrontCounterClockwise = TRUE;
    m_RasterizerDefault.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    m_RasterizerDefault.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    m_RasterizerDefault.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    m_RasterizerDefault.DepthClipEnable = TRUE;
    m_RasterizerDefault.MultisampleEnable = FALSE;
    m_RasterizerDefault.AntialiasedLineEnable = FALSE;
    m_RasterizerDefault.ForcedSampleCount = 0;
    m_RasterizerDefault.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    m_RasterizerDefaultCw = m_RasterizerDefault;
    m_RasterizerDefaultCw.FrontCounterClockwise = FALSE;

    m_RasterizerWireframe = m_RasterizerDefault;
    m_RasterizerWireframe.FillMode = D3D12_FILL_MODE_WIREFRAME;

    m_RasterizerWireframeCw = m_RasterizerWireframe;
    m_RasterizerWireframeCw.FrontCounterClockwise = FALSE;
}

void GraphicCommon::InitializeDepthStates()
{
    m_DepthStateDisabled.DepthEnable = FALSE;
    m_DepthStateDisabled.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    m_DepthStateDisabled.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    m_DepthStateDisabled.StencilEnable = FALSE;
    m_DepthStateDisabled.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    m_DepthStateDisabled.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    m_DepthStateDisabled.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    m_DepthStateDisabled.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    m_DepthStateDisabled.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    m_DepthStateDisabled.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    m_DepthStateDisabled.BackFace = m_DepthStateDisabled.FrontFace;

    m_DepthStateReadWrite = m_DepthStateDisabled;
    m_DepthStateReadWrite.DepthEnable = TRUE;
    m_DepthStateReadWrite.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    m_DepthStateReadWrite.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    m_DepthStateReadOnly = m_DepthStateReadWrite;
    m_DepthStateReadOnly.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

    m_DepthStateTestEqual = m_DepthStateReadOnly;
    m_DepthStateTestEqual.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL;
}

void GraphicCommon::InitializeBlendingStates()
{
    D3D12_BLEND_DESC alphaBlend = {};
    alphaBlend.IndependentBlendEnable = FALSE;
    alphaBlend.RenderTarget[0].BlendEnable = FALSE;
    alphaBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    alphaBlend.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    alphaBlend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    alphaBlend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    alphaBlend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
    alphaBlend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    alphaBlend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    m_BlendDisabled = alphaBlend;

    alphaBlend.RenderTarget[0].BlendEnable = TRUE;
    m_BlendTraditional = alphaBlend;

    alphaBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    m_BlendPreMultiplied = alphaBlend;

    alphaBlend.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
    m_BlendAdditive = alphaBlend;

    alphaBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    m_BlendTraditionalAdditive = alphaBlend;
}

void GraphicCommon::InitializeRootSignatures()
{
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    m_DefaultRootSignature = std::make_unique<RootSignature>(2, 0);

    (*m_DefaultRootSignature)[0].SetAsConstant(4, 0, D3D12_SHADER_VISIBILITY_ALL);
    (*m_DefaultRootSignature)[1].SetAsConstant(16, 1, D3D12_SHADER_VISIBILITY_ALL);
    m_DefaultRootSignature->Finalize(L"Default Root Signature", rootSignatureFlags);
}

D3D12_INPUT_ELEMENT_DESC inputElementDesc[2] = 
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

void GraphicCommon::InitializeShaders()
{
    m_DefaultInputLayout.NumElements = 2;
    m_DefaultInputLayout.pInputElementDescs = inputElementDesc;

    m_DefaultVS = std::make_unique<Shader>(L"vs_plexus.hlsl", L"VS_Main", L"vs_6_0", ShaderType::SHADERTYPE_VS, m_DefaultInputLayout);
    m_DefaultPS = std::make_unique<Shader>(L"ps_plexus.hlsl", L"PS_Main", L"ps_6_0", ShaderType::SHADERTYPE_PS, m_DefaultInputLayout);

    m_DefaultVS->Compile();
    m_DefaultPS->Compile();
}

void GraphicCommon::InitializePipelineStates()
{
    m_DefaultPSO = std::make_unique<GraphicPipelineState>(L"Default PSO");
    m_DefaultPSO->SetBlendState(m_BlendDisabled);
    m_DefaultPSO->SetRasterizerState(m_RasterizerDefault);
    m_DefaultPSO->SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    m_DefaultPSO->SetVertexShader(m_DefaultVS->GetCompiledShader(), m_DefaultVS->GetCompiledShaderSize());
    m_DefaultPSO->SetPixelShader(m_DefaultPS->GetCompiledShader(), m_DefaultPS->GetCompiledShaderSize());
    m_DefaultPSO->SetInputLayout(m_DefaultVS->GetInputLayout());
    m_DefaultPSO->SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
    m_DefaultPSO->SetDepthTargetFormat(DXGI_FORMAT_D32_FLOAT);
    m_DefaultPSO->SetDepthStencilState(m_DepthStateReadWrite);
    m_DefaultPSO->SetSamplingDesc(1, 0);
    m_DefaultPSO->SetRootSignature(*m_DefaultRootSignature);
    m_DefaultPSO->SetSampleMask(0xFFFFFFFF);
    m_DefaultPSO->Finalize();

    m_DefaultWireframePSO = std::make_unique<GraphicPipelineState>(L"Default Wireframe PSO");
    m_DefaultWireframePSO->SetBlendState(m_BlendDisabled);
    m_DefaultWireframePSO->SetRasterizerState(m_RasterizerWireframe);
    m_DefaultWireframePSO->SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    m_DefaultWireframePSO->SetVertexShader(m_DefaultVS->GetCompiledShader(), m_DefaultVS->GetCompiledShaderSize());
    m_DefaultWireframePSO->SetPixelShader(m_DefaultPS->GetCompiledShader(), m_DefaultPS->GetCompiledShaderSize());
    m_DefaultWireframePSO->SetInputLayout(m_DefaultVS->GetInputLayout());
    m_DefaultWireframePSO->SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
    m_DefaultWireframePSO->SetDepthTargetFormat(DXGI_FORMAT_D32_FLOAT);
    m_DefaultWireframePSO->SetDepthStencilState(m_DepthStateReadWrite);
    m_DefaultWireframePSO->SetSamplingDesc(1, 0);
    m_DefaultWireframePSO->SetRootSignature(*m_DefaultRootSignature);
    m_DefaultWireframePSO->SetSampleMask(0xFFFFFFFF);
    m_DefaultWireframePSO->Finalize();
}

ETH_NAMESPACE_END

