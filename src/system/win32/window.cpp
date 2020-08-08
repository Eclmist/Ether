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

#include "window.h"
#include "engine/engine.h"
#include "imgui/imgui_impl_win32.h"

//#define ETH_STANDALONE

#define ETH_WINDOW_CLASS        L"Ether Direct3D Window Class"
#define ETH_WINDOW_ICON         L"../src/system/win32/ether.ico"
#define ETH_WINDOWCLASS_STYLE   CS_HREDRAW | CS_VREDRAW

#ifdef ETH_EDITOR_BUILD
#define ETH_WINDOW_STYLE        WS_CHILD
#else
#define ETH_WINDOW_STYLE        WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU
#endif

Window::Window(Engine* engine)
    : EngineSubsystem(engine)
{
    m_hWnd = nullptr;
    m_hInst = nullptr;
    m_IsFullscreen = false;
    m_WindowedRect = { 0, 0, 0, 0 };
}

void Window::Initialize()
{
    // Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
    // Using this awareness context allows the client area of the window 
    // to achieve 100% scaling while still allowing non-client window content to 
    // be rendered in a DPI sensitive fashion.
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    RegisterWindowClass();

    CentralizeClientRect(
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        m_Engine->GetEngineConfig().GetClientWidth(),
        m_Engine->GetEngineConfig().GetClientHeight()
    );

    AdjustWindowRect(&m_WindowedRect, ETH_WINDOW_STYLE, FALSE);

    m_hWnd = CreateWindowExW(
        NULL,
        ETH_WINDOW_CLASS,
        m_Engine->GetEngineConfig().GetWindowTitle(),
        ETH_WINDOW_STYLE,
        m_WindowedRect.left,
        m_WindowedRect.top,
        m_WindowedRect.right - m_WindowedRect.left,
        m_WindowedRect.bottom - m_WindowedRect.top,
        m_Engine->GetEngineConfig().GetEditorHwndHost(),
        nullptr,
        m_hInst,
        this
    );

    ImGui_ImplWin32_Init(m_hWnd);

    SetInitialized(true);
}

void Window::Shutdown()
{
    ImGui_ImplWin32_Shutdown();
    DestroyWindow(m_hWnd);
    UnregisterClassW(ETH_WINDOW_CLASS, m_hInst);
}

void Window::Run()
{
    Show();

    // Lock thread and let WinProc handle the rest.
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

void Window::Show()
{
    ShowWindow(m_hWnd, SW_SHOW);
}

void Window::ToggleFullscreen()
{
#ifdef ETH_EDITOR_BUILD
    // Toggling of fullscreen is currently not supported when the window's hWnd
    // is a child of another. TODO: Fix this.
    return;
#endif

    if (m_IsFullscreen) // Go to windowed mode
    {
        SetWindowLongW(m_hWnd, GWL_STYLE, ETH_WINDOW_STYLE);
        SetViewportRect(m_WindowedRect);
        ShowWindow(m_hWnd, SW_NORMAL);
        m_IsFullscreen = false;
    }
    else // Go to fullscreen mode
    {
        // Cache windowed rect
        GetWindowRect(m_hWnd, &m_WindowedRect);
        RECT monitorRect = GetCurrentMonitorRect();
        SetWindowLongW(m_hWnd, GWL_STYLE, WS_OVERLAPPED);
        SetViewportRect(monitorRect);
        ShowWindow(m_hWnd, SW_MAXIMIZE);
        m_IsFullscreen = true;
    }
}

void Window::SetViewportRect(RECT viewportRect)
{
    RECT windowRect = viewportRect;
    AdjustWindowRect(&windowRect, ETH_WINDOW_STYLE, FALSE);

    SetWindowPos(
        m_hWnd,
        HWND_TOP,
        windowRect.left,
        windowRect.top,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        SWP_FRAMECHANGED | SWP_NOACTIVATE);

    // TODO: Find more elegant method to trigger resize
    m_Engine->GetRenderer()->Resize(viewportRect.right - viewportRect.left, viewportRect.bottom - viewportRect.top);
}

void Window::CentralizeClientRect(int screenWidth, int screenHeight, int clientWidth, int clientHeight)
{
    int clientPosX = std::max<int>(0, (screenWidth - clientWidth) / 2);
    int clientPosY = std::max<int>(0, (screenHeight - clientHeight) / 2);

    m_WindowedRect.left = clientPosX;
    m_WindowedRect.right = clientPosX + clientWidth;
    m_WindowedRect.top = clientPosY;
    m_WindowedRect.bottom = clientPosY + clientHeight;
}

RECT Window::GetCurrentMonitorRect() const
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

void Window::RegisterWindowClass() const
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

LRESULT CALLBACK Window::WndProcSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_NCCREATE)
    {
        const CREATESTRUCTW* cstruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
        Window* ethWnd = static_cast<Window*>(cstruct->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ethWnd));
        return ethWnd->WndProcInternal(hWnd, msg, wParam, lParam);
    }
    else
    {
        Window* ethWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        return ethWnd->WndProcInternal(hWnd, msg, wParam, lParam);
    }
}

LRESULT Window::WndProcInternal(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // ImGui should read the message first.
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_PAINT:
        // TODO: Figure out how to populate args
        RenderEventArgs renderArgs;
        m_Engine->OnRender(renderArgs);
        break;
    case WM_KEYDOWN:
        // TODO: Handle Ctrl, Alt, and convert keycode to character
        KeyEventArgs keydownArgs;
        keydownArgs.m_Key = static_cast<KeyCode>(wParam);
        keydownArgs.m_State = KeyEventArgs::KEYSTATE_KEYDOWN;
        m_Engine->OnKeyPressed(keydownArgs);
        break;
    case WM_KEYUP:
        KeyEventArgs keyupArgs;
        keyupArgs.m_Key = static_cast<KeyCode>(wParam);
        keyupArgs.m_State = KeyEventArgs::KEYSTATE_KEYUP;
        m_Engine->OnKeyReleased(keyupArgs);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    return 0;
}
