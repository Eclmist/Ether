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

ETH_NAMESPACE_END

