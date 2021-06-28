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

#include "api.h"
#include "graphic/graphic.h"
#include "system/win32/window.h"

ETH_NAMESPACE_BEGIN

Logger g_Logger;
Renderer g_Renderer;

void InitializeApplication()
{
    g_Logger.Initialize();
    g_Renderer.Initialize();
    g_MainApplication->Initialize();
}

void TerminateApplication()
{
    g_MainApplication->Shutdown();
    g_Renderer.Shutdown();
    g_Logger.Shutdown();
}

bool UpdateApplication()
{
    g_Renderer.Render();

    g_MainApplication->Update();
    g_MainApplication->RenderScene();
    g_MainApplication->RenderGui();
    return !g_MainApplication->IsDone();
}

int Start(ApplicationBase& app, HINSTANCE hInst, int cmdShow)
{
    LogInfo("Starting Ether v%d.%d.%d", 0, 1, 0);

    g_MainApplication = &app;
    g_MainApplication->Configure();

    Win32::Window gameWindow(app.GetClientName(), hInst);

    InitializeApplication();
    gameWindow.Show(cmdShow);

    do
    {
        MSG msg = {};
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (msg.message == WM_QUIT)
            break;
    } 
    while (UpdateApplication());

    TerminateApplication();
    return 0;
}

ETH_NAMESPACE_END

