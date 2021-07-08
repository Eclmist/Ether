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

ETH_NAMESPACE_BEGIN

Win32::Window g_MainWindow;

EngineConfig g_EngineConfig;
LoggingManager g_LoggingManager;
GraphicManager g_GraphicManager;
World g_World;

void InitializeEngine()
{
    Win32::g_CommandLineOptions.Initialize();
    g_MainApplication->Initialize();
    g_MainWindow.Initialize();
    g_LoggingManager.Initialize();
    g_GraphicManager.Initialize();
}

void TerminateEngine()
{
    g_GraphicManager.Shutdown();
    g_LoggingManager.Shutdown();
    g_MainApplication->Shutdown();
    g_MainWindow.Shutdown();
}

bool UpdateEngine()
{
    g_MainApplication->OnUpdate(UpdateEventArgs());

    RenderEventArgs renderArgs;
    renderArgs.m_TotalElapsedTime = GetTimeSinceStart();
    renderArgs.m_GraphicContext = &g_GraphicManager.GetGraphicContext();
    g_MainApplication->OnRender(renderArgs);

    g_GraphicManager.WaitForPresent();
    g_GraphicManager.Render();
    g_GraphicManager.RenderGui();
    g_GraphicManager.Present();

    return !g_MainApplication->ShouldExit();
}

void WindowsMessageLoop()
{
    do
    {
        Input::GetInstance().NewFrame();

        MSG msg = {};
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (msg.message == WM_QUIT)
            break;
    } 
    while (UpdateEngine());
}

int Start(ApplicationBase& app, int cmdShow)
{
    LogInfo("Starting Ether v%d.%d.%d", 0, 1, 0);

    g_MainApplication = &app;

    g_MainApplication->LoadContent();
    InitializeEngine();
    g_MainWindow.Show(cmdShow);

    WindowsMessageLoop();
    TerminateEngine();
    return 0;
}

ETH_NAMESPACE_END

