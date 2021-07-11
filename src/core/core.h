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

#include <cassert>
#include <deque>
#include <queue>
#include <string>
#include <vector>

#include "coredefinitions.h"

#include "system/noncopyable.h"
#include "system/singleton.h"
#include "system/stringutils.h"
#include "system/types.h"
#include "system/win32/keycodes.h"

#include "core/component/transformcomponent.h"
#include "core/component/meshcomponent.h"
#include "core/component/visualcomponent.h"

#include "core/event/events.h"
#include "core/engineconfig.h"
#include "core/entity.h"
#include "core/world.h"
#include "core/material.h"
#include "core/applicationbase.h"
#include "core/input.h"

#include "common/logging/loggingmanager.h"
#include "common/time.h"

ETH_NAMESPACE_BEGIN

ETH_ENGINE_DLL extern ApplicationBase* g_MainApplication;
ETH_ENGINE_DLL extern EngineConfig g_EngineConfig;
ETH_ENGINE_DLL extern LoggingManager g_LoggingManager;
ETH_ENGINE_DLL extern World g_World;

ETH_NAMESPACE_END

