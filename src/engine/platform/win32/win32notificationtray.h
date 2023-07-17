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

#pragma once

#include "engine/pch.h"
#include "engine/platform/platformnotificationtray.h"
#include "engine/platform/win32/ethwin.h"

namespace Ether::Win32
{
class Win32NotificationTray : public PlatformNotificationTray
{
public:
    Win32NotificationTray();
    ~Win32NotificationTray() override;

private:
    void AddTrayIcon();
    void RemoveTrayIcon();
    static LRESULT CALLBACK SysTrayWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
} // namespace Ether::Win32
