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

D3D12_RASTERIZER_DESC g_RasterizerDefault;
D3D12_RASTERIZER_DESC g_RasterizerDefaultCw;
D3D12_RASTERIZER_DESC g_RasterizerWireframe;
D3D12_RASTERIZER_DESC g_RasterizerWireframeCw;

D3D12_BLEND_DESC g_BlendDisabled;
D3D12_BLEND_DESC g_BlendPreMultiplied;
D3D12_BLEND_DESC g_BlendTraditional;
D3D12_BLEND_DESC g_BlendAdditive;
D3D12_BLEND_DESC g_BlendTraditionalAdditive;

D3D12_DEPTH_STENCIL_DESC g_DepthStateDisabled;
D3D12_DEPTH_STENCIL_DESC g_DepthStateReadWrite;
D3D12_DEPTH_STENCIL_DESC g_DepthStateReadOnly;
D3D12_DEPTH_STENCIL_DESC g_DepthStateTestEqual;

D3D12_INPUT_LAYOUT_DESC g_DefaultInputLayout;
D3D12_INPUT_ELEMENT_DESC g_DefaultInputElements[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

Shader* g_DefaultPS;
Shader* g_DefaultVS;

GraphicPipelineState* g_DefaultPSO;
GraphicPipelineState* g_DefaultWireframePSO;

RootSignature* g_DefaultRootSignature;

void InitializeRasterizerStates()
{
    g_RasterizerDefault.FillMode = D3D12_FILL_MODE_SOLID;
    g_RasterizerDefault.CullMode = D3D12_CULL_MODE_BACK;
    g_RasterizerDefault.FrontCounterClockwise = TRUE;
    g_RasterizerDefault.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    g_RasterizerDefault.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    g_RasterizerDefault.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    g_RasterizerDefault.DepthClipEnable = TRUE;
    g_RasterizerDefault.MultisampleEnable = FALSE;
    g_RasterizerDefault.AntialiasedLineEnable = FALSE;
    g_RasterizerDefault.ForcedSampleCount = 0;
    g_RasterizerDefault.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    g_RasterizerDefaultCw = g_RasterizerDefault;
    g_RasterizerDefaultCw.FrontCounterClockwise = FALSE;

    g_RasterizerWireframe = g_RasterizerDefault;
    g_RasterizerWireframe.FillMode = D3D12_FILL_MODE_WIREFRAME;

    g_RasterizerWireframeCw = g_RasterizerWireframe;
    g_RasterizerWireframeCw.FrontCounterClockwise = FALSE;
}

void InitializeDepthStates()
{
    g_DepthStateDisabled.DepthEnable = FALSE;
    g_DepthStateDisabled.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    g_DepthStateDisabled.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    g_DepthStateDisabled.StencilEnable = FALSE;
    g_DepthStateDisabled.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    g_DepthStateDisabled.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    g_DepthStateDisabled.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    g_DepthStateDisabled.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    g_DepthStateDisabled.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    g_DepthStateDisabled.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    g_DepthStateDisabled.BackFace = g_DepthStateDisabled.FrontFace;

    g_DepthStateReadWrite = g_DepthStateDisabled;
    g_DepthStateReadWrite.DepthEnable = TRUE;
    g_DepthStateReadWrite.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    g_DepthStateReadWrite.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    g_DepthStateReadOnly = g_DepthStateReadWrite;
    g_DepthStateReadOnly.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

    g_DepthStateTestEqual = g_DepthStateReadOnly;
    g_DepthStateTestEqual.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL;
}

void InitializeBlendingStates()
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
    g_BlendDisabled = alphaBlend;

    alphaBlend.RenderTarget[0].BlendEnable = TRUE;
    g_BlendTraditional = alphaBlend;

    alphaBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    g_BlendPreMultiplied = alphaBlend;

    alphaBlend.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
    g_BlendAdditive = alphaBlend;

    alphaBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    g_BlendTraditionalAdditive = alphaBlend;
}

void InitializeRootSignatures()
{
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    g_DefaultRootSignature = new RootSignature(2, 0);

    (*g_DefaultRootSignature)[0].SetAsConstant(4, 0, D3D12_SHADER_VISIBILITY_ALL);
    (*g_DefaultRootSignature)[1].SetAsConstant(16, 1, D3D12_SHADER_VISIBILITY_ALL);
    g_DefaultRootSignature->Finalize(L"Default Root Signature", rootSignatureFlags);
}

void InitializeShaders()
{
    g_DefaultVS = new Shader(L"vs_default.hlsl", L"VS_Main", L"vs_6_0", ShaderType::SHADERTYPE_VS);
    g_DefaultPS = new Shader(L"ps_default.hlsl", L"PS_Main", L"ps_6_0", ShaderType::SHADERTYPE_PS);

    g_DefaultVS->Compile();
    g_DefaultPS->Compile();

    g_DefaultInputLayout.NumElements = 2;
    g_DefaultInputLayout.pInputElementDescs = g_DefaultInputElements;
}

void InitializePipelineStates()
{
    g_DefaultPSO = new GraphicPipelineState(L"Default PSO");
    g_DefaultPSO->SetBlendState(g_BlendDisabled);
    g_DefaultPSO->SetRasterizerState(g_RasterizerDefault);
    g_DefaultPSO->SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    g_DefaultPSO->SetVertexShader(g_DefaultVS->GetCompiledShader(), g_DefaultVS->GetCompiledShaderSize());
    g_DefaultPSO->SetPixelShader(g_DefaultPS->GetCompiledShader(), g_DefaultPS->GetCompiledShaderSize());
    g_DefaultPSO->SetInputLayout(g_DefaultVS->GetInputLayout());
    g_DefaultPSO->SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
    g_DefaultPSO->SetDepthTargetFormat(DXGI_FORMAT_D32_FLOAT);
    g_DefaultPSO->SetDepthStencilState(g_DepthStateReadWrite);
    g_DefaultPSO->SetSamplingDesc(1, 0);
    g_DefaultPSO->SetRootSignature(*g_DefaultRootSignature);
    g_DefaultPSO->SetSampleMask(0xFFFFFFFF);
    g_DefaultPSO->Finalize();

    g_DefaultWireframePSO = new GraphicPipelineState(L"Default Wireframe PSO");
    g_DefaultWireframePSO->SetBlendState(g_BlendDisabled);
    g_DefaultWireframePSO->SetRasterizerState(g_RasterizerWireframe);
    g_DefaultWireframePSO->SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    g_DefaultWireframePSO->SetVertexShader(g_DefaultVS->GetCompiledShader(), g_DefaultVS->GetCompiledShaderSize());
    g_DefaultWireframePSO->SetPixelShader(g_DefaultPS->GetCompiledShader(), g_DefaultPS->GetCompiledShaderSize());
    g_DefaultWireframePSO->SetInputLayout(g_DefaultVS->GetInputLayout());
    g_DefaultWireframePSO->SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
    g_DefaultWireframePSO->SetDepthTargetFormat(DXGI_FORMAT_D32_FLOAT);
    g_DefaultWireframePSO->SetDepthStencilState(g_DepthStateReadWrite);
    g_DefaultWireframePSO->SetSamplingDesc(1, 0);
    g_DefaultWireframePSO->SetRootSignature(*g_DefaultRootSignature);
    g_DefaultWireframePSO->SetSampleMask(0xFFFFFFFF);
    g_DefaultWireframePSO->Finalize();
}

void DestroyRootSignatures()
{
    delete g_DefaultRootSignature;
}

void DestroyShaders()
{
    delete g_DefaultVS;
    delete g_DefaultPS;
}

void DestroyPipelineStates()
{
    delete g_DefaultPSO;
    delete g_DefaultWireframePSO;
}

void InitializeCommonStates()
{
    InitializeRasterizerStates();
    InitializeDepthStates();
    InitializeBlendingStates();
    InitializeRootSignatures();
    InitializeShaders();
    InitializePipelineStates();
}

void DestroyCommonStates()
{
    DestroyRootSignatures();
    DestroyShaders();
    DestroyPipelineStates();
}

ETH_NAMESPACE_END

