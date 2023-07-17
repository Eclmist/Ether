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

#include "engine/pch.h"

#include "api/iapplicationbase.h"
#include "engine/config/engineconfig.h"
#include "engine/config/commandlineoptions.h"
#include "engine/platform/platformwindow.h"
#include "engine/world/world.h"

#include "graphics/graphiccore.h"

namespace Ether
{
class EngineCore : public Singleton<EngineCore>
{
public:
    EngineCore() = default;
    ~EngineCore() = default;

    void Initialize();
    void LoadApplication(IApplicationBase& app);
    void Run();
    void Shutdown();

public:
    static inline EngineConfig& GetEngineConfig() { return Instance().m_EngineConfig; }
    static inline CommandLineOptions& GetCommandLineOptions() { return Instance().m_CommandLineOptions; }
    static inline PlatformWindow& GetMainWindow() { return *Instance().m_MainWindow; }
    static inline World& GetActiveWorld() { return *Instance().m_ActiveWorld; }

    static inline bool IsInitialized() { return Instance().m_IsInitialized; }

private:
    void InitializeGraphicsLayer();
    void MainEngineThread();

private:
    bool m_IsInitialized = false;
    std::thread m_MainEngineThread;

    std::unique_ptr<PlatformWindow> m_MainWindow;
    std::unique_ptr<World> m_ActiveWorld;

    IApplicationBase* m_MainApplication = nullptr;
    EngineConfig m_EngineConfig;
    CommandLineOptions m_CommandLineOptions;
};
} // namespace Ether
