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

void InitializeCommonStates()
{
    InitializeRasterizerStates();
    InitializeDepthStates();
    InitializeBlendingStates();
}

ETH_NAMESPACE_END

