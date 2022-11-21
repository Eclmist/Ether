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

#include "engine/platform/platformwindow.h"
#include "engine/platform/win32/ethwin.h"

namespace Ether::Win32
{
    typedef SMath::Rect<long> Rect;

    class Win32Window : public PlatformWindow
    {
    public:
        Win32Window();
        ~Win32Window();

        void Show() override;
        void Hide() override;

        void SetClientSize(ethVector2u size) override;
        void SetClientPosition(ethVector2u pos) override;
        void SetFullscreen(bool isFullscreen) override;
        void SetTitle(const std::string& title) override;
        void SetParentWindowHandle(void* parentHandle) override;

    public:
		void PlatformMessageLoop(std::function<void()> engineUpdateCallback) override;

    private:
        Rect GetCurrentWindowRect();
        void ToWindowRect(Rect& clientRect);
        void ToClientRect(Rect& windowRect);
        void CentralizeWindow();
        void RegisterWindowClass() const;
        static LRESULT CALLBACK WndProcSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        LRESULT WndProcInternal(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    private:
        std::string m_ClassName;
		std::thread m_OsEventThread;
    };
}

