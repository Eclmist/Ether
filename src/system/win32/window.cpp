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
#include "core/engine.h"
#include "imgui/imgui_impl_win32.h"

ETH_NAMESPACE_BEGIN

namespace Win32
{

#define ETH_WINDOWCLASS_STYLE   CS_HREDRAW | CS_VREDRAW
#define ETH_WINDOW_STYLE        WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU

HWND g_hWnd = nullptr;

Window::Window(const wchar_t* classname, HINSTANCE hInst)
    : m_ClassName(classname)
    , m_hInst(hInst)
    , m_IsFullscreen(false)
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
        classname,
        classname,
        ETH_WINDOW_STYLE,
        m_WindowRect.left,
        m_WindowRect.top,
        m_WindowRect.right - m_WindowRect.left,
        m_WindowRect.bottom - m_WindowRect.top,
        nullptr,
        nullptr,
        hInst,
        this
    );
}

Window::~Window()
{
    DestroyWindow(g_hWnd);
    UnregisterClassW(m_ClassName, m_hInst);
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
    windowClass.hInstance = m_hInst;
    windowClass.hCursor = nullptr;
    windowClass.hbrBackground = nullptr;
    windowClass.lpszMenuName = nullptr;
    windowClass.lpszClassName = m_ClassName;
    windowClass.hIconSm = nullptr;
    windowClass.hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_ENGINEICON));

    if (RegisterClassExW(&windowClass) == 0)
    {
        LogWin32Fatal("Failed to register Window Class");
        assert(false && "Failed to register Window Class");
    }
}

void Window::PositionWindowRect()
{
    HWND desktopHwnd = GetDesktopWindow();
    RECT screenRect;
    GetWindowRect(desktopHwnd, &screenRect);

    m_WindowRect.left = ((screenRect.right - screenRect.left) / 2 - g_MainApplication->GetClientWidth() / 2);
    m_WindowRect.top = ((screenRect.bottom - screenRect.top) / 2 - g_MainApplication->GetClientHeight() / 2);
    m_WindowRect.right = m_WindowRect.left + g_MainApplication->GetClientWidth();
    m_WindowRect.bottom = m_WindowRect.top + g_MainApplication->GetClientHeight();
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
        Engine::GetInstance().OnKeyPressed(keydownArgs);
        break;
    case WM_KEYUP:
        KeyEventArgs keyupArgs;
        keyupArgs.m_Key = static_cast<KeyCode>(wParam);
        keyupArgs.m_State = KeyEventArgs::KEYSTATE_KEYUP;
        Engine::GetInstance().OnKeyReleased(keyupArgs);
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