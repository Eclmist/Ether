/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2023 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "rhimodule.h"

#ifdef ETH_GRAPHICS_DX12
#include "dx12/dx12module.h"
#endif

#ifdef ETH_GRAPHICS_AGC
#include "agc/agcmodule.h"
#endif

ETH_NAMESPACE_BEGIN

RhiModuleHandle RhiModule::CreateModule()
{
#if defined(ETH_GRAPHICS_DX12)
    return RhiModuleHandle(new Dx12Module());
#else defined (ETH_GRAPHICS_AGC)
    return RhiModuleHandle(new AgcModule());
#endif

    LogGraphicsFatal("No supported graphics API defined");
    return RhiModuleHandle();
}

ETH_NAMESPACE_END
