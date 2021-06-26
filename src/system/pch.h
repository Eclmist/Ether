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

// Namespace Utils
enum { InEtherNamespace = false };
namespace Ether { enum { InEtherNamespace = true }; }
#define ETH_NAMESPACE_BEGIN      static_assert(!InEtherNamespace, "Ether namespace not previously closed"); namespace Ether {
#define ETH_NAMESPACE_END        } static_assert(!InEtherNamespace, "Ether namespace not previously opened");

#define ETH_EXPORT_DLL __declspec(dllexport)
#define ETH_IMPORT_DLL __declspec(dllimport)

// Win32
#include "win32/ethwin.h"

// STL Headers
#include <algorithm>
#include <cassert>
#include <string>
#include <memory>
#include <mutex>

// ComPtr library
#include <wrl.h>
namespace wrl = Microsoft::WRL;

// Chrono
#include <chrono>
namespace chrono = std::chrono;

// Common Containers
#include <vector>
#include <list>
#include <stack>
#include <queue>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <map>

// Resources
#include "../resource/resource.h"

// Useful Ether includes
#include "noncopyable.h"
#include "singleton.h"
#include "types.h"

#include "common/logger.h"
#include "common/time.h"

// Globals
ETH_NAMESPACE_BEGIN

// Win32
extern HWND g_hWnd;

// Engine Subsystems
class GfxRenderer;
extern GfxRenderer* g_GfxRenderer;

ETH_NAMESPACE_END
