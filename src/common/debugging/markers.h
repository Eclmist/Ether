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

#ifdef ETH_PLATFORM_WIN32
    #define ETH_MARKER_FRAME(name) \
        OPTICK_FRAME(name);
    #define ETH_MARKER_THREAD(name) \
        OPTICK_THREAD(name);
    #define ETH_MARKER_EVENT(name) \
        OPTICK_EVENT(name);
#else
    #define ETH_MARKER_FRAME(name) 
    #define ETH_MARKER_THREAD(name) 
    #define ETH_MARKER_EVENT(name) 
#endif



