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

#include "toolmode/toolmain.h"
#include "toolmode/ipc/ipcmanager.h"
#include "engine/platform/win32/ethwin.h"

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int cmdShow)
{
    CreateMutex(0, false, "Local\\Ether.Toolmode");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        MessageBox(nullptr, "An instance of Ether is already running.", "Ether", MB_OK | MB_ICONEXCLAMATION);
        return EXIT_FAILURE;
    }

    Ether::Toolmode::EtherHeadless etherHeadless;
    return Start(etherHeadless);
}


void Ether::Toolmode::EtherHeadless::Initialize()
{
}

void Ether::Toolmode::EtherHeadless::LoadContent()
{
    World& world = GetActiveWorld();
    world.Load("D:\\Graphics_Projects\\Atelier\\Workspaces\\Debug\\TestScene.ether");
}

void Ether::Toolmode::EtherHeadless::UnloadContent()
{
}

void Ether::Toolmode::EtherHeadless::Shutdown()
{
}

void Ether::Toolmode::EtherHeadless::OnUpdate(const Ether::UpdateEventArgs& e)
{
    IpcManager::Instance().ProcessIncomingCommands();
    IpcManager::Instance().ProcessOutgoingCommands();
}

void Ether::Toolmode::EtherHeadless::OnRender(const Ether::RenderEventArgs& e)
{
}

void Ether::Toolmode::EtherHeadless::OnShutdown()
{
}
