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

#define ETH_WINDOWCLASS_STYLE               CS_HREDRAW | CS_VREDRAW

#ifdef ETH_TOOLMODE
#define ETH_WINDOW_STYLE                    WS_CHILD
#else
#define ETH_WINDOW_STYLE                    WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU
#endif

#define ETH_WINDOW_STYLE_FULLSCREEN         WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)

#include <strsafe.h>
#include <shellapi.h>
#include <winuser.h>

Window::Window()
    : m_HwndParent(ETH_ENGINE_OR_TOOL(nullptr, EngineCore::GetEditorHwnd()))
    , m_IsFullscreen(false)
{
    // Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
    // Using this awareness context allows the client area of the window 
    // to achieve 100% scaling while still allowing non-client window content to 
    // be rendered in a DPI sensitive fashion.
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    RegisterWindowClass();
    CenterWindowRect();
    AdjustWindowRect(&m_WindowedRect, ETH_WINDOW_STYLE, FALSE);

    ETH_TOOLONLY(AssertWin32(m_HwndParent != nullptr, "Toolmode cannot be run without a parent HWND"));

    m_Hwnd = CreateWindowExW(
        NULL,
        EngineCore::GetEngineConfig().GetClientName().c_str(),
        EngineCore::GetEngineConfig().GetClientName().c_str(),
        ETH_WINDOW_STYLE,
        m_WindowedRect.left,
        m_WindowedRect.top,
        m_WindowedRect.right - m_WindowedRect.left,
        m_WindowedRect.bottom - m_WindowedRect.top,
        m_HwndParent,
        nullptr,
        GetModuleHandle(NULL),
        this
    );

    AssertWin32(m_Hwnd != nullptr, "Failed to create a Win32 handle");

    ShowNotificationIcon();
}

Window::~Window()
{
    DestroyWindow(m_Hwnd);
    UnregisterClassW(EngineCore::GetEngineConfig().GetClientName().c_str(), GetModuleHandle(NULL));
}

void Window::Show(int cmdShow)
{
    ShowWindow(m_Hwnd, cmdShow);
}

void Window::ToggleFullscreen()
{
    m_IsFullscreen = !m_IsFullscreen;

    if (m_IsFullscreen)
    {
        SetWindowLongW(m_Hwnd, GWL_STYLE, ETH_WINDOW_STYLE_FULLSCREEN);
        GetWindowRect(m_Hwnd, &m_WindowedRect);
        SetWindowPos(m_Hwnd, HWND_TOP,
            0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
            SWP_FRAMECHANGED | SWP_NOACTIVATE);
        ShowWindow(m_Hwnd, SW_MAXIMIZE);
    }
    else
    {
        SetWindowLong(m_Hwnd, GWL_STYLE, ETH_WINDOW_STYLE);
        SetWindowPos(m_Hwnd, HWND_TOP,
            m_WindowedRect.left, m_WindowedRect.top, 
            m_WindowedRect.right - m_WindowedRect.left,
            m_WindowedRect.bottom - m_WindowedRect.top,
            SWP_FRAMECHANGED | SWP_NOACTIVATE);
        ShowWindow(m_Hwnd, SW_NORMAL);
    }
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

void Window::CenterWindowRect()
{
#ifdef ETH_TOOLMODE
    m_WindowedRect.left = 0;
    m_WindowedRect.top = 0;
    m_WindowedRect.right = EngineCore::GetEngineConfig().GetClientWidth();
    m_WindowedRect.bottom = EngineCore::GetEngineConfig().GetClientHeight();
#else
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    m_WindowedRect.left = (screenWidth / 2 - EngineCore::GetEngineConfig().GetClientWidth() / 2);
    m_WindowedRect.top = (screenHeight / 2 - EngineCore::GetEngineConfig().GetClientHeight() / 2);
    m_WindowedRect.right = m_WindowedRect.left + EngineCore::GetEngineConfig().GetClientWidth();
    m_WindowedRect.bottom = m_WindowedRect.top + EngineCore::GetEngineConfig().GetClientHeight();
#endif
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

#define WM_TRAY (WM_USER + 100)
#define IDM_EXIT 1001

LRESULT Window::WndProcInternal(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (!EngineCore::IsInitialized() || !GraphicCore::IsInitialized())
        return DefWindowProcW(hWnd, msg, wParam, lParam);

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
    case WM_TRAY:
    {
        switch (LOWORD(lParam))
        {
            case WM_RBUTTONUP:
            {
                POINT pt;
                GetCursorPos(&pt);
                HMENU hmenu = CreatePopupMenu();
                InsertMenu(hmenu, 0, MF_BYPOSITION | MF_STRING, IDM_EXIT, L"exit");
                TrackPopupMenu(hmenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, 0, GetActiveWindow(), NULL);
                break;
            }
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

void Window::ShowNotificationIcon()
{
    NOTIFYICONDATA nid = { sizeof(nid) };
    nid.hWnd = m_Hwnd;
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP | NIF_GUID;
    nid.uCallbackMessage = WM_TRAY;
    nid.uID = IDI_ENGINEICON;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ENGINEICON));
    nid.uVersion = NOTIFYICON_VERSION_4;
    lstrcpy(nid.szTip, EngineCore::GetEngineConfig().GetClientName().c_str());

    Shell_NotifyIcon(NIM_ADD, &nid);
    Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

ETH_NAMESPACE_END

