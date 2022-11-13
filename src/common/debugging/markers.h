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

#include "optick/optick.h"
#include "pix/pix3.h"

ETH_NAMESPACE_BEGIN

#define ETH_MARKER_FRAME(name) \
    OPTICK_FRAME(name); \
    PIXScopedEvent(PIX_COLOR_INDEX(1), name)

#define ETH_MARKER_THREAD(name) \
    OPTICK_THREAD(name); \
    PIXScopedEvent(PIX_COLOR_INDEX(2), name)

#define ETH_MARKER_EVENT(name) \
    OPTICK_EVENT(name); \
    PIXScopedEvent(PIX_COLOR_INDEX(3), name)

ETH_NAMESPACE_END
