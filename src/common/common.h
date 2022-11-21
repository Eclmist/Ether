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

#include <cassert>
#include <cstdint>
#include <string>
#include <limits>

#include "common/commondefinitions.h"
#include "common/debugging/markers.h"

#include "common/utils/noncopyable.h"
#include "common/utils/nonmovable.h"
#include "common/utils/singleton.h"
#include "common/utils/types.h"
#include "common/utils/time.h"
#include "common/utils/exceptions.h"
#include "common/utils/stringid.h"

#include "common/memory/memoryutils.h"
