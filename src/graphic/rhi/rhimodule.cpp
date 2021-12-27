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

#include "rhimodule.h"

#define ETH_HAS_D3D12_SUPPORT

#ifdef ETH_HAS_D3D12_SUPPORT
#include "d3d12/d3d12module.h"
#endif

#ifdef ETH_HAS_VULKAN_SUPPORT
#include "vulkan/vulkanmodule.h"
#endif

ETH_NAMESPACE_BEGIN

RHIModuleHandle RHIModule::CreateModule(RHIModuleType type)
{
    if (type == RHIModuleType::D3D12)
#ifdef ETH_HAS_D3D12_SUPPORT
        return RHIModuleHandle(new D3D12Module());
#else
        LogGraphicsFatal("D3D12 is not supported on the current platform");
#endif

    if (type == RHIModuleType::Vulkan)
#ifdef ETH_HAS_VULKAN_SUPPORT
        return RHIModuleHandle(new VulkanModule());
#else
        LogGraphicsFatal("Vulkan is not yet supported");
#endif

    return RHIModuleHandle();
}

ETH_NAMESPACE_END
