/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2023 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "window.h"
#include "imgui/imgui_impl_win32.h"
#include <strsafe.h>
#include <shellapi.h>
#include <winuser.h>

ETH_NAMESPACE_BEGIN

#define ETH_WINDOWCLASS_STYLE               CS_HREDRAW | CS_VREDRAW

#ifdef ETH_TOOLMODE
#define ETH_WINDOW_STYLE                    WS_POPUP
#else
#define ETH_WINDOW_STYLE                    WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME
#endif

#define ETH_WINDOW_STYLE_FULLSCREEN         WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)

namespace Win32
{

Window::Window()
    : m_WindowedRect({0, 0, 1920, 1080})
{
    // Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
    // Using this awareness context allows the client area of the window 
    // to achieve 100% scaling while still allowing non-client window content to 
    // be rendered in a DPI sensitive fashion.
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    RegisterWindowClass();
    ETH_ENGINEONLY(CenterWindowRect());
    ETH_ENGINEONLY(AdjustWindowRect(&m_WindowedRect, ETH_WINDOW_STYLE, FALSE));

    m_WindowHandle = (void*)CreateWindowExW(
        WS_EX_NOREDIRECTIONBITMAP,
        EngineCore::GetEngineConfig().GetClientName().c_str(),
        EngineCore::GetEngineConfig().GetClientName().c_str(),
        ETH_WINDOW_STYLE,
        m_WindowedRect.left,
        m_WindowedRect.top,
        m_WindowedRect.right - m_WindowedRect.left,
        m_WindowedRect.bottom - m_WindowedRect.top,
        nullptr,
        nullptr,
        GetModuleHandle(NULL),
        this
    );

    AssertWin32(m_WindowHandle != nullptr, "Failed to create a Win32 handle");
}

Window::~Window()
{
    DestroyWindow((HWND)m_WindowHandle);
    UnregisterClassW(EngineCore::GetEngineConfig().GetClientName().c_str(), GetModuleHandle(NULL));
}

void Window::Show()
{
    ShowWindow((HWND)m_WindowHandle, SW_SHOW);
}

void Window::Hide()
{
    ShowWindow((HWND)m_WindowHandle, SW_HIDE);
}

void Window::SetSize(uint32_t width, uint32_t height)
{
    m_WindowedRect.right = m_WindowedRect.left + width;
    m_WindowedRect.bottom = m_WindowedRect.top + height;
    MoveWindow((HWND)m_WindowHandle, m_WindowedRect.left, m_WindowedRect.top, width, height, true);
}

void Window::SetFullscreen(bool isFullscreen)
{
    if (isFullscreen)
    {
        SetWindowLongW((HWND)m_WindowHandle, GWL_STYLE, ETH_WINDOW_STYLE_FULLSCREEN);
        GetWindowRect((HWND)m_WindowHandle, &m_WindowedRect);
        SetWindowPos((HWND)m_WindowHandle, HWND_TOP,
            0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
            SWP_FRAMECHANGED | SWP_NOACTIVATE);
        ShowWindow((HWND)m_WindowHandle, SW_MAXIMIZE);
    }
    else
    {
        SetWindowLong((HWND)m_WindowHandle, GWL_STYLE, ETH_WINDOW_STYLE);
        SetWindowPos((HWND)m_WindowHandle, HWND_TOP,
            m_WindowedRect.left, m_WindowedRect.top,
            m_WindowedRect.right - m_WindowedRect.left,
            m_WindowedRect.bottom - m_WindowedRect.top,
            SWP_FRAMECHANGED | SWP_NOACTIVATE);
        ShowWindow((HWND)m_WindowHandle, SW_NORMAL);
    }
}

void Window::SetParentWindowHandle(void* parentHandle)
{
    m_ParentWindowHandle = parentHandle;
    SetWindowLong((HWND)m_WindowHandle, GWL_STYLE, m_ParentWindowHandle == nullptr ? ETH_WINDOW_STYLE : WS_CHILD);
    SetParent((HWND)m_WindowHandle, (HWND)m_ParentWindowHandle);
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
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    m_WindowedRect.left = (screenWidth / 2 - EngineCore::GetEngineConfig().GetClientWidth() / 2);
    m_WindowedRect.top = (screenHeight / 2 - EngineCore::GetEngineConfig().GetClientHeight() / 2);
    m_WindowedRect.right = m_WindowedRect.left + EngineCore::GetEngineConfig().GetClientWidth();
    m_WindowedRect.bottom = m_WindowedRect.top + EngineCore::GetEngineConfig().GetClientHeight();
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
    if (!EngineCore::IsInitialized() || !GraphicCore::IsInitialized())
        return DefWindowProcW(hWnd, msg, wParam, lParam);

    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_MOUSEACTIVATE:
        ETH_TOOLONLY(SetFocus((HWND)m_WindowHandle));
        break;
    case WM_SIZE:
        EngineCore::GetEngineConfig().SetClientWidth(LOWORD(lParam));
        EngineCore::GetEngineConfig().SetClientHeight(HIWORD(lParam));
        GraphicCore::GetGraphicDisplay().QueueBufferResize(LOWORD(lParam), HIWORD(lParam));
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
        EngineCore::GetMainApplication().ScheduleExit();
        break;
    case WM_ERASEBKGND:
        return 1;
    default:
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    return 0;
}
}

ETH_NAMESPACE_END

