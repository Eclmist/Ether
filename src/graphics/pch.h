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

#pragma once

#include "common/common.h"
#include "common/logging/loggingmanager.h"
#include "common/time/time.h"

#include "graphics/rhi/rhitypes.h"
#include "graphics/rhi/rhiresource.h"

namespace Ether::Graphics
{
    constexpr uint32_t MaxSwapChainBuffers          = 3;
    constexpr uint32_t DefaultBackBufferWidth       = 1920;
    constexpr uint32_t DefaultBackBufferHeight      = 1080;
    constexpr RhiFormat BackBufferFormat            = RhiFormat::R8G8B8A8Unorm;
    constexpr RhiFormat DepthBufferFormat           = RhiFormat::D24UnormS8Uint;

    constexpr size_t _1KiB                          = 1 << 10;
    constexpr size_t _2KiB                          = 1 << 11;
    constexpr size_t _4KiB                          = 1 << 12;
    constexpr size_t _8KiB                          = 1 << 13;
    constexpr size_t _16KiB                         = 1 << 14;
    constexpr size_t _32KiB                         = 1 << 15;
    constexpr size_t _64KiB                         = 1 << 16;
    constexpr size_t _128KiB                        = 1 << 17;
    constexpr size_t _256KiB                        = 1 << 18;
    constexpr size_t _512KiB                        = 1 << 19;
    constexpr size_t _1MiB                          = 1 << 20;
    constexpr size_t _2MiB                          = 1 << 21;
    constexpr size_t _4MiB                          = 1 << 22;
    constexpr size_t _8MiB                          = 1 << 23;
    constexpr size_t _16MiB                         = 1 << 24;
    constexpr size_t _32MiB                         = 1 << 25;
    constexpr size_t _64MiB                         = 1 << 26;
}

