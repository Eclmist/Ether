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
#include "imgui/imgui_impl_win32.h"

ETH_NAMESPACE_BEGIN

namespace Win32
{

HWND g_hWnd = nullptr;

#define ETH_WINDOWCLASS_STYLE   CS_HREDRAW | CS_VREDRAW
#define ETH_WINDOW_STYLE        WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU

#include <strsafe.h>

void Window::Initialize()
{
    // Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
    // Using this awareness context allows the client area of the window 
    // to achieve 100% scaling while still allowing non-client window content to 
    // be rendered in a DPI sensitive fashion.
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    RegisterWindowClass();
    PositionWindowRect();
    AdjustWindowRect(&m_WindowRect, ETH_WINDOW_STYLE, FALSE);

    g_hWnd = CreateWindowExW(
        NULL,
        g_EngineConfig.GetClientName().c_str(),
        g_EngineConfig.GetClientName().c_str(),
        ETH_WINDOW_STYLE,
        m_WindowRect.left,
        m_WindowRect.top,
        m_WindowRect.right - m_WindowRect.left,
        m_WindowRect.bottom - m_WindowRect.top,
        nullptr,
        nullptr,
        GetModuleHandle(NULL),
        this
    );
}

void Window::Shutdown()
{
    DestroyWindow(g_hWnd);
    UnregisterClassW(g_EngineConfig.GetClientName().c_str(), GetModuleHandle(NULL));
}

void Window::Show(int cmdShow)
{
    ShowWindow(g_hWnd, cmdShow);
}

void Window::RegisterWindowClass() const
{
    WNDCLASSEXW windowClass;

    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = ETH_WINDOWCLASS_STYLE;
    windowClass.lpfnWndProc = &WndProcSetup;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = GetModuleHandle(NULL);
    windowClass.hCursor = nullptr;
    windowClass.hbrBackground = nullptr;
    windowClass.lpszMenuName = nullptr;
    windowClass.lpszClassName = g_EngineConfig.GetClientName().c_str();
    windowClass.hIconSm = nullptr;
    windowClass.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ENGINEICON));

    AssertWin32(RegisterClassExW(&windowClass) != 0, "Failed to register Window Class");
}

void Window::PositionWindowRect()
{
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    m_WindowRect.left = (screenWidth / 2 - g_EngineConfig.GetClientWidth() / 2);
    m_WindowRect.top = (screenHeight / 2 - g_EngineConfig.GetClientHeight() / 2);
    m_WindowRect.right = m_WindowRect.left + g_EngineConfig.GetClientWidth();
    m_WindowRect.bottom = m_WindowRect.top + g_EngineConfig.GetClientHeight();
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
        //RenderEventArgs renderArgs;
        //m_Engine->OnRender(renderArgs);
        break;
    case WM_KEYDOWN:
        // TODO: Handle Ctrl, Alt, and convert keycode to character
        KeyEventArgs keydownArgs;
        keydownArgs.m_Key = static_cast<KeyCode>(wParam);
        keydownArgs.m_State = KeyEventArgs::KEYSTATE_KEYDOWN;
        g_MainApplication->OnKeyPress(keydownArgs);

        // TODO: Move into camera class
        if (keydownArgs.m_Key == KEYCODE_Q)
            g_GraphicManager.cameraY += 0.1;
        if (keydownArgs.m_Key == KEYCODE_E)
            g_GraphicManager.cameraY -= 0.1;
        if (keydownArgs.m_Key == KEYCODE_A)
            g_GraphicManager.cameraX += 0.1;
        if (keydownArgs.m_Key == KEYCODE_D)
            g_GraphicManager.cameraX -= 0.1;

        break;
    case WM_KEYUP:
        KeyEventArgs keyupArgs;
        keyupArgs.m_Key = static_cast<KeyCode>(wParam);
        keyupArgs.m_State = KeyEventArgs::KEYSTATE_KEYUP;
        g_MainApplication->OnKeyRelease(keydownArgs);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    return 0;
}

}

ETH_NAMESPACE_END
