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

#include "system/platform/win32/window.h"
#include "system/platform/win32/notificationtray.h"

#ifdef ETH_TOOLMODE
#include "toolmode/ipc/command/initcommand.h"
#endif

ETH_NAMESPACE_BEGIN

void EngineCore::Initialize(IApplicationBase& app)
{
    // Logger needs to be the very first thing to be initialized, as logs that were made
    // prior might be lost if the engine stalls or crashes before initialization!
    Instance().m_LoggingManager = std::make_unique<LoggingManager>();

    app.Initialize();
    Instance().m_MainApplication = &app;

#ifdef ETH_TOOLMODE
    Instance().m_NotificationTray = std::make_unique<Win32::NotificationTray>();
    Instance().m_IpcManager = std::make_unique<IpcManager>();
    Instance().m_IpcManager->WaitForEditor();
#endif

    Instance().m_MainWindow = std::make_unique<Win32::Window>();

#ifdef ETH_TOOLMODE
    auto initResponse = std::make_shared<InitCommandResponse>((void*)Instance().m_MainWindow->GetWindowHandle());
    Instance().m_IpcManager->QueueResponseCommand(initResponse);
#endif

    Instance().m_ActiveWorld = std::make_unique<World>();
    Instance().m_IsInitialized = true;
}

void EngineCore::LoadContent()
{
    // Other engine content can be loaded here before or after main application
    Instance().m_MainApplication->LoadContent();
}

void EngineCore::Shutdown()
{
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

