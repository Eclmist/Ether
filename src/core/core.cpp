/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "system/platform/win32/window.h"
#include "system/platform/win32/notificationtray.h"

ETH_NAMESPACE_BEGIN

void EngineCore::Initialize(IApplicationBase& app)
{
    app.Initialize();
    Instance().m_MainApplication = &app;
    Instance().m_LoggingManager = std::make_unique<LoggingManager>();
    Instance().m_MainWindow = std::make_unique<Win32::Window>();
    Instance().m_ActiveWorld = std::make_unique<World>();
    Instance().m_EcsManager = std::make_unique<EcsManager>();
    Instance().m_EcsManager->OnInitialize(); // TODO: This looks ugly, can we move this away?

#ifdef ETH_TOOLMODE
    Instance().m_NotificationTray = std::make_unique<Win32::NotificationTray>();
    Instance().m_AssetCompiler = std::make_unique<AssetCompiler>();
    Instance().m_IpcManager = std::make_unique<IpcManager>();
#endif

    Instance().m_IsInitialized = true;
}

void EngineCore::LoadContent()
{
    // Other engine content can be loaded here before or after main application
    Instance().m_MainApplication->LoadContent();

    // Initialize all resources that needs to be initialized during scene load
    // TODO: Replace with scene loading system in the future
    Instance().m_EcsManager->OnSceneLoad();
}

void EngineCore::Update()
{
    OPTICK_EVENT("Engine Core - Update");
    Instance().m_EcsManager->OnUpdate();
}

void EngineCore::Shutdown()
{
    Instance().m_EcsManager.reset();
    Instance().m_ActiveWorld.reset();
    Instance().m_MainWindow.reset();
    Instance().m_LoggingManager.reset();
    Reset();
}

bool EngineCore::IsInitialized()
{
    return HasInstance() && Instance().m_IsInitialized;
}

ETH_NAMESPACE_END

