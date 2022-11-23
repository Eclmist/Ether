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

#ifdef ETH_PLATFORM_WIN32

#include "engine/engine.h"
#include "engine/platform/win32/win32window.h"
#include "graphics/rhi/dx12/dx12imguiwrapper.h"
#include "resource.h"
#include <strsafe.h>
#include <shellapi.h>
#include <winuser.h>

#if defined UNICODE
#define ETH_WINDOW_CLASS                    L"Ether"
#else
#define ETH_WINDOW_CLASS                    "Ether"
#endif

#define ETH_WINDOWCLASS_STYLE               CS_HREDRAW | CS_VREDRAW

#ifdef ETH_TOOLMODE
#define ETH_WINDOW_STYLE                    WS_POPUP
#else
#define ETH_WINDOW_STYLE                    WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME
#endif

#define ETH_WINDOW_STYLE_FULLSCREEN         WS_VISIBLE | WS_POPUP

Ether::Win32::Win32Window::Win32Window()
    : m_QuitMessageReceived(false)
{
    RegisterWindowClass();
    m_WindowHandle = (void*)::CreateWindowEx(
        WS_EX_NOREDIRECTIONBITMAP,
        ETH_WINDOW_CLASS,
        ETH_WINDOW_CLASS,
        ETH_WINDOW_STYLE,
        0,
        0,
        1366,
        768,
        nullptr,
        nullptr,
        ::GetModuleHandle(nullptr),
        this
    );
    AssertWin32(m_WindowHandle != nullptr, "Failed to create a Win32 handle");

#if defined(ETH_ENGINE)
    CentralizeWindow();
#endif

}

Ether::Win32::Win32Window::~Win32Window()
{
    ::DestroyWindow((HWND)m_WindowHandle);
    UnregisterClass(ETH_WINDOW_CLASS, ::GetModuleHandle(nullptr));
}

void Ether::Win32::Win32Window::Show()
{
    if (m_WindowHandle == nullptr)
        return;

    ::ShowWindow((HWND)m_WindowHandle, SW_SHOW);
}

void Ether::Win32::Win32Window::Hide()
{
    if (m_WindowHandle == nullptr)
        return;

    ::ShowWindow((HWND)m_WindowHandle, SW_HIDE);
}

void Ether::Win32::Win32Window::SetClientSize(ethVector2u size)
{
    if (m_WindowHandle == nullptr)
        return;

    Rect windowRect = GetCurrentWindowRect();
    Rect newRect = { 0, 0, (long)size.x, (long)size.y };
    ToWindowRect(newRect);

    if (windowRect.w == newRect.w && windowRect.h == newRect.h)
        return;

    ::MoveWindow((HWND)m_WindowHandle, windowRect.x, windowRect.y, newRect.w, newRect.h, false);
}

void Ether::Win32::Win32Window::SetClientPosition(ethVector2u pos)
{
    if (m_WindowHandle == nullptr)
        return;

    Rect windowRect = GetCurrentWindowRect();
    ::SetWindowPos((HWND)m_WindowHandle, 0, pos.x, pos.y, windowRect.w, windowRect.h, 0);
}

void Ether::Win32::Win32Window::SetFullscreen(bool isFullscreen)
{
    if (m_WindowHandle == nullptr)
        return;

    m_IsFullscreen = isFullscreen;
    static Rect previousRect = { 0,0,0,0 };

    auto style = isFullscreen ? WS_POPUP : ETH_WINDOW_STYLE;
    auto showFlag = isFullscreen ? SW_MAXIMIZE : SW_NORMAL;
    auto posFlag = SWP_FRAMECHANGED | SWP_NOACTIVATE;
    Rect newRect = isFullscreen 
        ? Rect(0, 0, GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN)) 
        : previousRect;

    previousRect = GetCurrentWindowRect();

    ::SetWindowLong((HWND)m_WindowHandle, GWL_STYLE, style);
    ::SetWindowPos((HWND)m_WindowHandle, HWND_TOP, newRect.x, newRect.y, newRect.w, newRect.h, posFlag);
    ::ShowWindow((HWND)m_WindowHandle, showFlag);
}

void Ether::Win32::Win32Window::SetTitle(const std::string& title)
{
    if (m_WindowHandle == nullptr)
        return;

#if defined UNICODE
    ::SetWindowText((HWND)m_WindowHandle, ToWideString(title).c_str());
#else
    ::SetWindowText((HWND)m_WindowHandle, title.c_str());
#endif
}

void Ether::Win32::Win32Window::SetParentWindowHandle(void* parentHandle)
{
    if (m_WindowHandle == nullptr)
        return;

    m_ParentWindowHandle = parentHandle;
    ::SetWindowLong((HWND)m_WindowHandle, GWL_STYLE, m_ParentWindowHandle == nullptr ? ETH_WINDOW_STYLE : WS_CHILD);
    ::SetParent((HWND)m_WindowHandle, (HWND)m_ParentWindowHandle);
}

void Ether::Win32::Win32Window::PlatformMessageLoop(std::function<void()> engineUpdateCallback)
{
    while (true)
    {
        MSG msg = {};
        if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }

        if (m_QuitMessageReceived || msg.message == WM_QUIT)
            return;

        engineUpdateCallback();
    }
}

void Ether::Win32::Win32Window::ToWindowRect(Rect& clientRect)
{
    RECT windowRect = { clientRect.x, clientRect.y, clientRect.x + clientRect.w, clientRect.y + clientRect.h };
    ::AdjustWindowRect(&windowRect, m_IsFullscreen ? ETH_WINDOW_STYLE_FULLSCREEN : ETH_WINDOW_STYLE, false);

    clientRect = {
        windowRect.left,
        windowRect.top,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top
    };
}

void Ether::Win32::Win32Window::ToClientRect(Rect& windowRect)
{
    RECT rc;
    ::SetRectEmpty(&rc);
    ::AdjustWindowRect(&rc, m_IsFullscreen ? ETH_WINDOW_STYLE_FULLSCREEN : ETH_WINDOW_STYLE, false);

    windowRect.w -= (rc.right - rc.left);
    windowRect.h -= (rc.bottom - rc.top);
    windowRect.x += rc.left;
    windowRect.y += rc.top;
}

Ether::Win32::Rect Ether::Win32::Win32Window::GetCurrentWindowRect()
{
    if (m_WindowHandle == nullptr)
        return { 0,0,0,0 };

    RECT currentWindowRect;
    ::GetWindowRect((HWND)m_WindowHandle, &currentWindowRect);

    return {
        currentWindowRect.left,
        currentWindowRect.top,
        currentWindowRect.right - currentWindowRect.left,
        currentWindowRect.bottom - currentWindowRect.top
    };
}

void Ether::Win32::Win32Window::CentralizeWindow()
{
    if (m_WindowHandle == nullptr)
        return;

    Rect windowRect = GetCurrentWindowRect();
    int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

    windowRect.x = (screenWidth / 2 - windowRect.w / 2);
    windowRect.y = (screenHeight / 2 - windowRect.h / 2);

    ::MoveWindow((HWND)m_WindowHandle, windowRect.x, windowRect.y, windowRect.w, windowRect.h, true);
}

void Ether::Win32::Win32Window::RegisterWindowClass() const
{
    WNDCLASSEX windowClass;

    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = ETH_WINDOWCLASS_STYLE;
    windowClass.lpfnWndProc = &WndProcInternal;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = ::GetModuleHandle(nullptr);
    windowClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = nullptr;
    windowClass.lpszMenuName = nullptr;
    windowClass.lpszClassName = ETH_WINDOW_CLASS;
    windowClass.hIconSm = nullptr;
    windowClass.hIcon = ::LoadIcon(::GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_ENGINEICON));

    AssertWin32(::RegisterClassEx(&windowClass) != 0, "Failed to register Window Class");
}

LRESULT CALLBACK Ether::Win32::Win32Window::WndProcInternal(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (!Engine::IsInitialized())
        return DefWindowProc(hWnd, msg, wParam, lParam);

    if (((Graphics::Dx12ImguiWrapper&)Graphics::Core::GetImguiWrapper()).Win32MessageHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_MOUSEACTIVATE:
        ETH_TOOLONLY(::SetFocus((HWND)m_WindowHandle));
        break;
    case WM_SIZE:
        if (Engine::GetEngineConfig().GetClientSize() != ethVector2u{ (uint32_t)LOWORD(lParam), (uint32_t)HIWORD(lParam) })
            Engine::GetEngineConfig().SetClientSize({ LOWORD(lParam), HIWORD(lParam) });
        break;
    case WM_MOVE:
    {
        Rect clientRect = { LOWORD(lParam), HIWORD(lParam), 0, 0 };
        ((Win32Window&)Engine::GetMainWindow()).ToClientRect(clientRect);
        ethVector2u clientPos = { (uint32_t)clientRect.x, (uint32_t)clientRect.y };

        if (Engine::GetEngineConfig().GetClientPosition() != clientPos)
            Engine::GetEngineConfig().SetClientPosition(clientPos);
        break;
    }
    case WM_KEYDOWN:
        Input::Instance().SetKeyDown(static_cast<Ether::KeyCode>(wParam));
        break;
    case WM_KEYUP:
        Input::Instance().SetKeyUp(static_cast<Ether::KeyCode>(wParam));
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
        ::PostQuitMessage(0);
        ((Win32Window&)Engine::GetMainWindow()).m_QuitMessageReceived = true;
        break;
    case WM_ERASEBKGND:
        return 1;
    default:
        return ::DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return 0;
}

#endif // ETH_PLATFORM_WIN32

