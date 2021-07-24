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
#include <mutex>
#include <unordered_map>
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

#include "core/window.h"
#include "core/commandlineoptions.h"
#include "core/event/events.h"
#include "core/engineconfig.h"
#include "core/entity.h"
#include "core/world.h"
#include "core/material.h"
#include "core/iapplicationbase.h"
#include "core/input.h"

#include "common/logging/loggingmanager.h"
#include "common/time.h"

ETH_NAMESPACE_BEGIN

class ETH_ENGINE_DLL EngineCore : public Singleton<EngineCore>
{
public:
    static void Initialize(IApplicationBase& app);
    static void LoadContent();
    static void Shutdown();
    static bool IsInitialized();

    static IApplicationBase& GetMainApplication() { return *Instance().m_MainApplication; }

    static LoggingManager& GetLoggingManager() { return *Instance().m_LoggingManager; }
    static Window& GetMainWindow() { return *Instance().m_MainWindow; }
    static World& GetActiveWorld() { return *Instance().m_ActiveWorld; }

    static EngineConfig& GetEngineConfig() { return Instance().m_EngineConfig; }
    static CommandLineOptions& GetCommandLineOptions() { return Instance().m_CommandLineOptions; }

    ETH_TOOLONLY(static HWND GetEditorHwnd() { return Instance().m_EditorHwnd; })
    ETH_TOOLONLY(static void SetEditorHwnd(HWND hwnd) { Instance().m_EditorHwnd = hwnd; })

private:
    IApplicationBase* m_MainApplication;

    std::unique_ptr<LoggingManager> m_LoggingManager;
    std::unique_ptr<World> m_ActiveWorld;
    std::unique_ptr<Window> m_MainWindow;

    CommandLineOptions m_CommandLineOptions;
    EngineConfig m_EngineConfig;

private:
    bool m_IsInitialized;
    ETH_TOOLONLY(HWND m_EditorHwnd;)
};

ETH_NAMESPACE_END

