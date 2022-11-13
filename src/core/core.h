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
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <queue>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <functional>

#include "coredefinitions.h"

#include "math/math.h"

#include "system/noncopyable.h"
#include "system/serializable.h"
#include "system/singleton.h"
#include "system/stringutils.h"
#include "system/types.h"

#include "system/platform/platformwindow.h"
#include "system/platform/platformnotificationtray.h"
#include "system/platform/win32/keycodes.h"

#include "core/asset/compiledmesh.h"
#include "core/asset/compiledtexture.h"
#include "core/asset/material.h"

#include "core/ecs/ecsmanager.h"
#include "core/world/world.h"

#include "core/commandlineoptions.h"
#include "core/event/events.h"
#include "core/engineconfig.h"
#include "core/iapplicationbase.h"
#include "core/input.h"

#include "common/hash.h"
#include "common/logging/loggingmanager.h"
#include "common/stream/stream.h"
#include "common/stringid.h"
#include "common/time.h"

#ifdef ETH_TOOLMODE
#include "toolmode/asset/assetcompiler.h"
#include "toolmode/ipc/ipcmanager.h"
#endif

#include "optick/optick.h"

ETH_NAMESPACE_BEGIN

class ETH_ENGINE_DLL EngineCore : public Singleton<EngineCore>
{
public:
    EngineCore() = default;
    ~EngineCore() = default;

    static void Initialize(IApplicationBase& app);
    static void LoadContent();
    static void Update();
    static void Shutdown();
    static bool IsInitialized();

    static IApplicationBase& GetMainApplication() { return *Instance().m_MainApplication; }
    static EcsManager& GetEcsManager() { return *Instance().m_EcsManager; }
    static LoggingManager& GetLoggingManager() { return *Instance().m_LoggingManager; }
    static PlatformWindow& GetMainWindow() { return *Instance().m_MainWindow; }
    static World& GetActiveWorld() { return *Instance().m_ActiveWorld; }
    ETH_TOOLONLY(static AssetCompiler& GetAssetCompiler() { return *Instance().m_AssetCompiler; })
    ETH_TOOLONLY(static IpcManager& GetIpcManager() { return *Instance().m_IpcManager; })

    static EngineConfig& GetEngineConfig() { return Instance().m_EngineConfig; }
    static CommandLineOptions& GetCommandLineOptions() { return Instance().m_CommandLineOptions; }

private:
    IApplicationBase* m_MainApplication;

    std::unique_ptr<EcsManager> m_EcsManager;
    std::unique_ptr<LoggingManager> m_LoggingManager;
    std::unique_ptr<World> m_ActiveWorld;
    std::unique_ptr<PlatformWindow> m_MainWindow;
    ETH_TOOLONLY(std::unique_ptr<PlatformNotificationTray> m_NotificationTray;)
    ETH_TOOLONLY(std::unique_ptr<AssetCompiler> m_AssetCompiler;)
    ETH_TOOLONLY(std::unique_ptr<IpcManager> m_IpcManager;)

    CommandLineOptions m_CommandLineOptions;
    EngineConfig m_EngineConfig;

private:
    bool m_IsInitialized;
};

ETH_NAMESPACE_END
