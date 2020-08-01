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

#include "windowmanager.h"
#include "engine/engine.h"
#include "graphic/gfxrenderer.h"
#include "imgui/imgui_impl_win32.h"

#define ETH_WINDOW_TITLE        L"Ether"
#define ETH_WINDOW_CLASS        L"Ether Direct3D Window Class"
#define ETH_WINDOW_ICON         L"../src/win32/ether.ico"
#define ETH_WINDOW_WIDTH        1270
#define ETH_WINDOW_HEIGHT       768
#define ETH_WINDOW_STYLE        WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU
#define ETH_WINDOWCLASS_STYLE   CS_HREDRAW | CS_VREDRAW

WindowManager::WindowManager(Engine* engine)
    : EngineSubsystem(engine)
{
}

void WindowManager::Initialize()
{
    m_hInst = GetModuleHandle(nullptr);

    // Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
    // Using this awareness context allows the client area of the window 
    // to achieve 100% scaling while still allowing non-client window content to 
    // be rendered in a DPI sensitive fashion.
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    RegisterWindowClass();

    CentralizeClientRect(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), ETH_WINDOW_WIDTH, ETH_WINDOW_HEIGHT);

    AdjustWindowRect(&m_WindowedRect, ETH_WINDOW_STYLE, FALSE);

    InitWindow();

    ImGui_ImplWin32_Init(m_hWnd);

    SetInitialized(true);
}

void WindowManager::InitializeForEditor()
{
    m_hWnd = m_Engine->GetEngineConfig().GetEditorHwnd();

    ImGui_ImplWin32_Init(m_hWnd);

    SetInitialized(true);
}

void WindowManager::Shutdown()
{
    ImGui_ImplWin32_Shutdown();
    DestroyWindow(m_hWnd);
    UnregisterClassW(ETH_WINDOW_CLASS, m_hInst);
}

void WindowManager::Show()
{
    ShowWindow(m_hWnd, SW_SHOW);
}

void WindowManager::SetFullscreen(bool isFullscreen)
{
    if (m_IsFullscreen == isFullscreen)
        return;

    if (isFullscreen)
    {
        RECT monitorRect = GetCurrentMonitorRect();
        GetWindowRect(m_hWnd, &m_WindowedRect);
        SetWindowLongW(m_hWnd, GWL_STYLE, WS_OVERLAPPED);
        SetWindowPos(
            m_hWnd,
            HWND_TOP,
            monitorRect.left,
            monitorRect.top,
            monitorRect.right - monitorRect.left,
            monitorRect.bottom - monitorRect.top,
            SWP_FRAMECHANGED | SWP_NOACTIVATE);
        ShowWindow(m_hWnd, SW_MAXIMIZE);
    }
    else
    {
        SetWindowLongW(m_hWnd, GWL_STYLE, ETH_WINDOW_STYLE);
        SetWindowPos(
            m_hWnd,
            HWND_NOTOPMOST,
            m_WindowedRect.left,
            m_WindowedRect.top,
            m_WindowedRect.right - m_WindowedRect.left,
            m_WindowedRect.bottom - m_WindowedRect.top,
            SWP_FRAMECHANGED | SWP_NOACTIVATE);
        ShowWindow(m_hWnd, SW_NORMAL);
    }

    m_IsFullscreen = isFullscreen;
}

void WindowManager::InitWindow()
{
    m_hWnd = CreateWindowExW(
        NULL,
        ETH_WINDOW_CLASS,
        ETH_WINDOW_TITLE,
        ETH_WINDOW_STYLE,
        m_WindowedRect.left,
        m_WindowedRect.top,
        m_WindowedRect.right - m_WindowedRect.left,
        m_WindowedRect.bottom - m_WindowedRect.top,
        nullptr,
        nullptr,
        m_hInst,
        this
    );
}

void WindowManager::CentralizeClientRect(int screenWidth, int screenHeight, int clientWidth, int clientHeight)
{
    int clientPosX = std::max<int>(0, (screenWidth - clientWidth) / 2);
    int clientPosY = std::max<int>(0, (screenHeight - clientHeight) / 2);

    m_WindowedRect.left = clientPosX;
    m_WindowedRect.right = clientPosX + clientWidth;
    m_WindowedRect.top = clientPosY;
    m_WindowedRect.bottom = clientPosY + clientHeight;
}

RECT WindowManager::GetCurrentMonitorRect() const
{
    // Query the name of the nearest display device for the window.
    // This is required to set the full screen dimensions of the window
    // when using a multi-monitor setup.
    HMONITOR hMonitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFOEX monitorInfo = {};
    monitorInfo.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo(hMonitor, &monitorInfo);

    return monitorInfo.rcMonitor;
}

void WindowManager::RegisterWindowClass() const
{
    WNDCLASSEXW windowClass;

    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = ETH_WINDOWCLASS_STYLE;
    windowClass.lpfnWndProc = &WndProcSetup;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = m_hInst;
    windowClass.hCursor = nullptr;
    windowClass.hbrBackground = nullptr;
    windowClass.lpszMenuName = nullptr;
    windowClass.lpszClassName = ETH_WINDOW_CLASS;
    windowClass.hIconSm = nullptr;
    windowClass.hIcon = (HICON)LoadImageW(
        m_hInst,
        ETH_WINDOW_ICON,
        IMAGE_ICON,
        0,
        0,
        LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED);

    if (RegisterClassExW(&windowClass) == 0)
    {
        assert(false && "Failed to register Window Class");
    }
}

LRESULT CALLBACK WindowManager::WndProcSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_NCCREATE)
    {
        const CREATESTRUCTW* cstruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
        WindowManager* ethWnd = static_cast<WindowManager*>(cstruct->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ethWnd));
        return ethWnd->WndProcInternal(hWnd, msg, wParam, lParam);
    }
    else
    {
        WindowManager* ethWnd = reinterpret_cast<WindowManager*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        return ethWnd->WndProcInternal(hWnd, msg, wParam, lParam);
    }
}

LRESULT WindowManager::WndProcInternal(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // ImGui should read the message first.
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_PAINT:
        m_Engine->GetRenderer()->RenderFrame();
        break;
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_F3:
            m_Engine->GetRenderer()->ToggleImGui();
            break;
        case VK_F11:
            SetFullscreen(!m_IsFullscreen);
            break;
        default:
            break;
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    return 0;
}
