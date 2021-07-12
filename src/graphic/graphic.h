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

#define ETH_MINIMUM_FEATURE_LEVEL           D3D_FEATURE_LEVEL_11_0

#define ETH_SHADER_DEBUG_DIR                L"../../src/graphic/shader/"
#define ETH_SHADER_RELEASE_DIR              L"./data/shader/"


#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)
#define D3D12_RESOURCE_STATE_UNKNOWN        ((D3D12_RESOURCE_STATES)-1)

#define ASSERT_SUCCESS(hr)                  if (FAILED(hr)) { LogGraphicsFatal("D3D12 operation failed - %s", #hr); throw std::exception(); }

// Ether Graphics Library
#include "graphic/graphiccore.h"
#include "graphic/commandmanager.h"
#include "graphic/graphicmanager.h"
#include "graphic/common/shaderdaemon.h"
#include "graphic/common/visual.h"

ETH_NAMESPACE_BEGIN

extern ID3D12Device3* g_GraphicDevice;

extern CommandManager g_CommandManager;
extern GraphicManager g_GraphicManager;
extern ShaderDaemon g_ShaderDaemon;


ETH_NAMESPACE_END

