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

#include "api.h"
#include "engine/engine.h"

int Ether::Start(IApplicationBase& app)
{
    Time::Instance().Initialize();
    Input::Instance().Initialize();
    LoggingManager::Instance().Initialize();

    LogInfo("Starting Ether v%d.%d.%d", 0, 1, 0);
    Engine::Instance().Initialize();
    Engine::Instance().LoadApplication(app);
    ETH_ENGINEONLY(Engine::Instance().GetMainWindow().Show());

    Engine::Instance().Run();

    LogInfo("Shutting down Ether"); 
    Engine::Instance().Shutdown();

    Input::Reset();
    Time::Reset();
    LoggingManager::Reset();
    return 0;
}

void Ether::Shutdown()
{
#ifdef ETH_PLATFORM_WIN32
    //PostQuitMessage(0);
#endif
}

Ether::CommandLineOptions& Ether::GetCommandLineOptions()
{
    return Engine::GetCommandLineOptions();
}

void Ether::Client::SetClientTitle(const std::string& title)
{
    Engine::GetEngineConfig().SetClientTitle(title);
}

std::string Ether::Client::GetClientTitle()
{
    return Engine::GetEngineConfig().GetClientName();
}

void Ether::Client::SetClientSize(const ethVector2u& size)
{
    Engine::GetEngineConfig().SetClientSize(size);
}

Ether::ethVector2u Ether::Client::GetClientSize()
{
    return Engine::GetEngineConfig().GetClientSize();
}
void Ether::Client::SetFullscreen(bool enabled)
{
    Engine::GetMainWindow().SetFullscreen(enabled);
}

bool Ether::Client::IsFullscreen()
{
    return Engine::GetMainWindow().IsFullscreen();
}

void Ether::Toolmode::SetParentWindow(void* hwnd)
{
    Engine::GetMainWindow().SetParentWindowHandle(hwnd);
}

void* Ether::Toolmode::GetWindowHandle()
{
    return Engine::GetMainWindow().GetWindowHandle();
}

void Ether::Toolmode::ShowWindow()
{
    Engine::GetMainWindow().Show();
}

void Ether::Toolmode::HideWindow()
{
    Engine::GetMainWindow().Hide();
}

