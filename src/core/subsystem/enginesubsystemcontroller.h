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

#include "core/config/engineconfig.h"
#include "system/win32/window.h"
#include "graphic/gfx/gfxrenderer.h"

class Engine;

class EngineSubsystemController : NonCopyable
{
public:
    EngineSubsystemController(Engine* engine);
    ~EngineSubsystemController() = default;

public:
    void InitializeSubsystems();
    void ShutdownSubsystems();

public:
    inline GfxRenderer* GetRenderer() const { return m_Renderer.get(); };

private:
    void InitializeRenderer();
    void ShutdownRenderer();

private:
    std::unique_ptr<GfxRenderer> m_Renderer;

    Engine* m_Engine;
};
