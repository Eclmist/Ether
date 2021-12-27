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

#define ETH_HAS_D3D12_SUPPORT

#ifdef ETH_HAS_D3D12_SUPPORT
#include "d3d12/d3d12includes.h"
#endif

#ifdef ETH_HAS_VULKAN_SUPPORT
#include "vulkan/vulkanincludes.h"
#endif

ETH_NAMESPACE_BEGIN

class RHIModule
{
public:
    RHIModule() = default;
	virtual ~RHIModule() = default;

public:
    virtual RHIResult Initialize() = 0;
    virtual RHIResult Shutdown() = 0;

public:
    virtual RHIResult CreateDevice(RHIDeviceHandle& device) const = 0;

public:
    static RHIModuleHandle CreateModule(RHIModuleType type);
};

ETH_NAMESPACE_END

