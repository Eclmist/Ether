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

#include "pch.h"
#include "engine/config/engineconfig.h"
#include <functional>

namespace Ether
{
class PlatformWindow : public NonCopyable, public NonMovable
{
public:
    PlatformWindow();
    virtual ~PlatformWindow() = default;

    virtual void Show() = 0;
    virtual void Hide() = 0;

    virtual void SetClientSize(const ethVector2u& size) = 0;
    virtual void SetClientPosition(const ethVector2u& pos) = 0;
    virtual void SetFullscreen(bool isFullscreen) = 0;
    virtual void SetParentWindowHandle(void* parentHandle) = 0;
    virtual void SetTitle(const std::string& title) = 0;

    inline bool IsFullscreen() const { return m_IsFullscreen; }

    inline void* GetWindowHandle() const { return m_WindowHandle; }
    inline void* GetParentWindowHandle() const { return m_ParentWindowHandle; }

public:
    virtual void PlatformMessageLoop() = 0;
    virtual bool ProcessPlatformMessages() = 0;

protected:
    bool m_IsFullscreen;

    void* m_WindowHandle;
    void* m_ParentWindowHandle;
};
} // namespace Ether
