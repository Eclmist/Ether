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

ETH_NAMESPACE_BEGIN

class PlatformWindow : public NonCopyable
{
public:
    PlatformWindow();
    virtual ~PlatformWindow() = default;

    virtual void Show() = 0;
    virtual void Hide() = 0;
    virtual void SetSize(uint32_t width, uint32_t height) = 0;
    virtual void SetFullscreen(bool isFullscreen) = 0;
    virtual void SetParentWindowHandle(void* parentHandle) = 0;

    inline bool GetFullscreen() const { return m_IsFullscreen; }

    inline void* GetWindowHandle() const { return m_WindowHandle; }
    inline void* GetParentWindowHandle() const { return m_ParentWindowHandle; }

protected:
    bool m_IsFullscreen;

    void* m_WindowHandle;
    void* m_ParentWindowHandle;
};


ETH_NAMESPACE_END

