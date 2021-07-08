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

// Win32
#include "system/win32/ethwin.h"

// STL Headers
#include <algorithm>
#include <string>
#include <memory>
#include <mutex>

// Chrono
#include <chrono>
namespace chrono = std::chrono;

// Common Containers
#include <vector>
#include <list>
#include <stack>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <map>

// Resources
#include "../resource/resource.h"

// Ether includes
#include "core/core.h"
#include "graphic/graphic.h"
#include "system/win32/window.h"
#include "system/win32/commandlineoptions.h"

ETH_NAMESPACE_BEGIN

namespace Win32
{
    extern HWND g_hWnd; 
    extern Window g_ClientWindow;
    extern CommandLineOptions g_CommandLineOptions;
}

ETH_NAMESPACE_END

