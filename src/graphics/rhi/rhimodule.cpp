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

#include "graphics/rhi/rhimodule.h"

#if defined (ETH_GRAPHICS_DX12)
#include "graphics/rhi/dx12/dx12module.h"
#elif defined (ETH_GRAPHICS_PS5)
#include "graphics/rhi/ps5/ps5module.h"
#endif

std::unique_ptr<Ether::Graphics::RhiModule> Ether::Graphics::RhiModule::InitForPlatform()
{
#if defined(ETH_GRAPHICS_DX12)
    return std::make_unique<Dx12Module>();
#else
    static_assert(false, "Not yet implemented");
#endif
}

