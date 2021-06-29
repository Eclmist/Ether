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

#include "coredefinitions.h"

#include <string>
#include <vector>

#include "system/noncopyable.h"
#include "system/singleton.h"
#include "system/types.h"

#include "core/event/events.h"
#include "core/config/engineconfig.h"
#include "core/applicationbase.h"

#include "common/logging/logger.h"
#include "common/time.h"

ETH_NAMESPACE_BEGIN

ETH_ENGINE_DLL extern ApplicationBase* g_MainApplication;
ETH_ENGINE_DLL extern EngineConfig g_EngineConfig;
ETH_ENGINE_DLL extern Logger g_Logger;

ETH_NAMESPACE_END

