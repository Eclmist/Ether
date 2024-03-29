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

#include "engine/platform/win32/win32notificationtray.h"
#include "engine/platform/win32/ethwin.h"
#include "resource.h"
#include <shellapi.h>

#define NOTIFICATION_TRAY_ICON_MSG (WM_USER + 0x100)
#define NOTIFICATION_TRAY_UID      1
#define IDM_EXIT                   100

Ether::Win32::Win32NotificationTray::Win32NotificationTray()
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = &SysTrayWndProc;
    wc.hInstance = ::GetModuleHandle(NULL);
    wc.lpszClassName = TEXT("Ether::NotificationTray");
    AssertWin32(RegisterClass(&wc) != 0, "Failed to register notification tray Window Class");

    m_Handle = (void*)CreateWindowEx(
        0,
        TEXT("Ether::NotificationTray"),
        NULL,
        0,
        0,
        0,
        0,
        0,
        HWND_MESSAGE,
        NULL,
        ::GetModuleHandle(NULL),
        NULL);

    if (!m_Handle)
    {
        LogWin32Error("Failed to create a Win32 handle for the notification tray icon");
        return;
    }

    AddTrayIcon();
}

Ether::Win32::Win32NotificationTray::~Win32NotificationTray()
{
    RemoveTrayIcon();
    ::DestroyWindow((HWND)m_Handle);
}

void Ether::Win32::Win32NotificationTray::AddTrayIcon()
{
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = (HWND)m_Handle;
    nid.uID = NOTIFICATION_TRAY_UID;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = NOTIFICATION_TRAY_ICON_MSG;
    nid.uVersion = NOTIFYICON_VERSION_4;
    nid.hIcon = ::LoadIcon(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ENGINEICON));

    if (::Shell_NotifyIcon(NIM_ADD, &nid))
        ::Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

void Ether::Win32::Win32NotificationTray::RemoveTrayIcon()
{
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = (HWND)m_Handle;
    nid.uID = NOTIFICATION_TRAY_UID;

    ::Shell_NotifyIcon(NIM_DELETE, &nid);
}

LRESULT CALLBACK Ether::Win32::Win32NotificationTray::SysTrayWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case NOTIFICATION_TRAY_ICON_MSG:
    {
        switch (LOWORD(lParam))
        {
        case NIN_SELECT:
        case NIN_KEYSELECT:
        case WM_CONTEXTMENU:
        {
            POINT pt;
            ::GetCursorPos(&pt);
            HMENU hmenu = ::CreatePopupMenu();
            ::InsertMenu(hmenu, 0, MF_BYPOSITION | MF_STRING, IDM_EXIT, "Force Quit (Debug)");
            ::SetForegroundWindow(hWnd);
            ::TrackPopupMenu(hmenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, NULL);
            ::PostMessage(hWnd, WM_NULL, 0, 0);
            break;
        }
        }

        return 0;
    }

    case WM_COMMAND:
        if (lParam == 0 && LOWORD(wParam) == IDM_EXIT)
            ::exit(EXIT_SUCCESS);
        break;
    }

    return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

#endif // ETH_PLATFORM_WIN32
