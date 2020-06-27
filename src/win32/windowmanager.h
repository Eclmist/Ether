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

#pragma once

#include "ethwin.h"
#include "system/system.h"
#include "system/subsystem.h"

class WindowManager : public SubSystem<WindowManager>
{
public:
    WindowManager(const wchar_t* windowTitle, int width = 1280, int height = 720);
    WindowManager(HWND hWnd);
    ~WindowManager();

public:
    void RegisterDependencies(SubSystemScheduler& schedule) override;

public:
    void Show();
    void SetFullscreen(bool isFullscreen);
    void InitWindow(const wchar_t* windowTitle);
    void CentralizeClientRect(int screenWidth, int screenHeight, int clientWidth, int clientHeight);
    RECT GetCurrentMonitorRect() const;
    void RegisterWindowClass() const noexcept;

    inline HWND GetHwnd() const { return m_hWnd; };
    inline uint32_t GetWidth() const { return m_WindowedRect.right - m_WindowedRect.left; };
    inline uint32_t GetHeight() const { return m_WindowedRect.bottom - m_WindowedRect.top; };

private:
    static LRESULT CALLBACK WndProcSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT WndProcInternal(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    // Handle toggling of fullscreen
    bool m_IsFullscreen;
    RECT m_WindowedRect;

    // Handle to the actual win32 window and window instance
    HWND m_hWnd;
    HINSTANCE m_hInst;
};