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

#define ETH_WINDOWCLASS_STYLE   CS_HREDRAW | CS_VREDRAW
#define ETH_WINDOW_STYLE        WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU

#include <strsafe.h>

Window::Window()
{
    // Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
    // Using this awareness context allows the client area of the window 
    // to achieve 100% scaling while still allowing non-client window content to 
    // be rendered in a DPI sensitive fashion.
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    RegisterWindowClass();
    PositionWindowRect();
    AdjustWindowRect(&m_WindowRect, ETH_WINDOW_STYLE, FALSE);

    m_hWnd = CreateWindowExW(
        NULL,
        EngineCore::GetEngineConfig().GetClientName().c_str(),
        EngineCore::GetEngineConfig().GetClientName().c_str(),
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

Window::~Window()
{
    DestroyWindow(m_hWnd);
    UnregisterClassW(EngineCore::GetEngineConfig().GetClientName().c_str(), GetModuleHandle(NULL));
}

void Window::Show(int cmdShow)
{
    ShowWindow(m_hWnd, cmdShow);
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
    windowClass.lpszClassName = EngineCore::GetEngineConfig().GetClientName().c_str();
    windowClass.hIconSm = nullptr;
    windowClass.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ENGINEICON));

    AssertWin32(RegisterClassExW(&windowClass) != 0, "Failed to register Window Class");
}

void Window::PositionWindowRect()
{
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    m_WindowRect.left = (screenWidth / 2 - EngineCore::GetEngineConfig().GetClientWidth() / 2);
    m_WindowRect.top = (screenHeight / 2 - EngineCore::GetEngineConfig().GetClientHeight() / 2);
    m_WindowRect.right = m_WindowRect.left + EngineCore::GetEngineConfig().GetClientWidth();
    m_WindowRect.bottom = m_WindowRect.top + EngineCore::GetEngineConfig().GetClientHeight();
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
    case WM_SIZE:
        EngineCore::GetEngineConfig().SetClientWidth(LOWORD(lParam));
        EngineCore::GetEngineConfig().SetClientHeight(HIWORD(lParam));
        GraphicCore::GetGraphicDisplay().Resize(LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_KEYDOWN:
        // TODO: Handle Ctrl, Alt, and convert keycode to character
        Input::Instance().SetKeyDown(static_cast<Win32::KeyCode>(wParam));
        break;
    case WM_KEYUP:
        Input::Instance().SetKeyUp(static_cast<Win32::KeyCode>(wParam));
        break;
    case WM_MOUSEWHEEL:
        Input::Instance().SetMouseWheelDelta(GET_WHEEL_DELTA_WPARAM(wParam));
        break;
    case WM_MOUSEMOVE:
        Input::Instance().SetMousePosX((short)LOWORD(lParam));
        Input::Instance().SetMousePosY((short)HIWORD(lParam));
        break;
    case WM_LBUTTONDOWN:
        Input::Instance().SetMouseButtonDown(0);
        break;
    case WM_LBUTTONUP:
        Input::Instance().SetMouseButtonUp(0);
        break;
    case WM_MBUTTONDOWN:
        Input::Instance().SetMouseButtonDown(1);
        break;
    case WM_MBUTTONUP:
        Input::Instance().SetMouseButtonUp(1);
        break;
    case WM_RBUTTONDOWN:
        Input::Instance().SetMouseButtonDown(2);
        break;
    case WM_RBUTTONUP:
        Input::Instance().SetMouseButtonUp(2);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    return 0;
}

ETH_NAMESPACE_END
