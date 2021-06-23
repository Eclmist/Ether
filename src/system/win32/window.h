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

namespace EtherGame
{
    class iGameApplication;
}

class Window
{
public:
    Window(EtherGame::iGameApplication& app, const wchar_t* classname, HINSTANCE hInst);
    ~Window();

public:
    void Show(int cmdShow);
    inline HWND GetHwnd() const { return m_hWnd; };

private:
    void RegisterWindowClass() const;
    static LRESULT CALLBACK WndProcSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT WndProcInternal(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    bool m_IsFullscreen;
    RECT m_WindowRect;

    // Handle to the actual win32 window and window instance
    HWND m_hWnd;
    HINSTANCE m_hInst;
    const wchar_t* m_ClassName;
};
