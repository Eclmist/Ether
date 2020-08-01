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

#include "engine.h"
#include "system/win32/windowmanager.h"

Engine::Engine(const EngineConfig& engineConfig)
    : m_EngineConfig(engineConfig)
    , m_SubsystemController(this)
{
}

void Engine::Initialize()
{
    m_SubsystemController.InitializeSubsystems();
}

void Engine::Run()
{
    m_SubsystemController.GetWindowManager()->Show();

    // Lock thread and let WinProc in WindowManager handle the rest.
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

void Engine::Shutdown()
{
    m_SubsystemController.ShutdownSubsystems();
}

