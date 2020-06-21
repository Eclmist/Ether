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

#include "win32/window.h"
#include "system/system.h"
#include "graphic/renderer.h"
#include "imgui/imguimanager.h"

uint32_t g_ClientWidth = 1280;
uint32_t g_ClientHeight = 720;

Window* g_Window;
Renderer* g_Renderer;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
    // TODO: Move this into an app class
    ImGuiManager imguiManager;

    g_Window = new Window(g_ClientWidth, g_ClientHeight, L"Ether");
    g_Renderer = new Renderer();

    // Renderer is an RAII type, dx12 should be initialized at this point.
    g_Window->Show();

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    g_Renderer->Flush();
    g_Renderer->Release();

    return 0;
}

ETH_EXPORT_DLL void Initialize(HWND hWnd)
{
    g_Window = new Window(hWnd);
    g_Renderer = new Renderer();
}

ETH_EXPORT_DLL void Update()
{
    g_Renderer->Render();
}

ETH_EXPORT_DLL void Release()
{
    g_Renderer->Flush();
    g_Renderer->Release();
    delete g_Renderer;
}

