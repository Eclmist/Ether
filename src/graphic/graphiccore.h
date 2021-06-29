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

// ComPtr library
#include <wrl.h>
namespace wrl = Microsoft::WRL;

// D3D12 library
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>

// D3D12 extension library
#include <d3dx12/d3dx12.h>

// Ether Graphics Library
#include "graphic/graphiccontext.h"
#include "graphic/graphicdisplay.h"
#include "graphic/resource/visual.h" // TODO: do we really need to expose this?