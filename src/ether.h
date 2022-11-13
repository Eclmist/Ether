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

// Absolute first thing to include. Since we are not sure were someone might include a <Window.h>
#include "system/platform/win32/ethwin.h"

// STL Headers
#include <algorithm>
#include <string>
#include <memory>

// Chrono
#include <chrono>
namespace chrono = std::chrono;

// Common Containers
#include <vector>
#include <list>
#include <stack>
#include <unordered_set>
#include <set>
#include <map>

// Resources
#include "../resource/resource.h"

// Ether includes
#include "core/core.h"
#include "graphic/graphic.h"

