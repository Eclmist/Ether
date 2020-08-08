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

#include "win32/ethwinmacros.h"

// STL Headers
#include <algorithm>
#include <cassert>
#include <string>
#include <memory>

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
#include <unordered_set>
#include <unordered_map>

// Useful standalone no-dep Ether includes
#include "noncopyable.h"
#include "singleton.h"
#include "types.h"
