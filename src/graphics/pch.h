/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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

#pragma once

#include "common/common.h"
#include "common/logging/loggingmanager.h"
#include "common/time/time.h"
#include "common/stream/filestream.h"
#include "common/stream/bytestream.h"

#include "graphics/rhi/rhitypes.h"
#include "graphics/rhi/rhiresource.h"
#include "graphics/rhi/rhiresourceviews.h"

#include "graphics/common/graphicenums.h"

namespace Ether::Graphics
{
    constexpr uint32_t MaxSwapChainBuffers          = 3;
    constexpr uint32_t DefaultBackBufferWidth       = 1920;
    constexpr uint32_t DefaultBackBufferHeight      = 1080;
    constexpr RhiFormat BackBufferLdrFormat         = RhiFormat::R8G8B8A8UnormSrgb; // Needs to be SRGB for auto gamma correction
    constexpr RhiFormat BackBufferHdrFormat         = RhiFormat::R32G32B32A32Float; // Will be copied to LDR buffer, so no need SRGB TODO: No need alpha component, use R32G32B32Float
    constexpr RhiFormat DepthBufferFormat           = RhiFormat::R24G8Typeless;
    constexpr RhiFormat DepthBufferDsvFormat        = RhiFormat::D24UnormS8Uint;
    constexpr RhiFormat DepthBufferSrvFormat        = RhiFormat::R24UNormX8Typeless;
}

