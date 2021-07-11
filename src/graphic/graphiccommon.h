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

#pragma once

ETH_NAMESPACE_BEGIN

void InitializeCommonStates();
void DestroyCommonStates();

extern D3D12_RASTERIZER_DESC g_RasterizerDefault;
extern D3D12_RASTERIZER_DESC g_RasterizerDefaultCw;
extern D3D12_RASTERIZER_DESC g_RasterizerWireframe;
extern D3D12_RASTERIZER_DESC g_RasterizerWireframeCw;

extern D3D12_BLEND_DESC g_BlendDisabled;
extern D3D12_BLEND_DESC g_BlendPreMultiplied;
extern D3D12_BLEND_DESC g_BlendTraditional;
extern D3D12_BLEND_DESC g_BlendAdditive;
extern D3D12_BLEND_DESC g_BlendTraditionalAdditive;

extern D3D12_DEPTH_STENCIL_DESC g_DepthStateDisabled;
extern D3D12_DEPTH_STENCIL_DESC g_DepthStateReadWrite;
extern D3D12_DEPTH_STENCIL_DESC g_DepthStateReadOnly;
extern D3D12_DEPTH_STENCIL_DESC g_DepthStateTestEqual;

extern D3D12_INPUT_LAYOUT_DESC g_DefaultInputLayout;

extern Shader* g_DefaultVS;
extern Shader* g_DefaultPS;

extern GraphicPipelineState* g_DefaultPSO;
extern GraphicPipelineState* g_DefaultWireframePSO;

extern RootSignature* g_DefaultRootSignature;

ETH_NAMESPACE_END

