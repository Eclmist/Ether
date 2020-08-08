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

#include "engine/config/engineconfig.h"
#include "system/win32/window.h"
#include "graphic/gfxrenderer.h"
#include "imgui/imguimanager.h"

class Engine;

class EngineSubsystemController
{
public:
    EngineSubsystemController(Engine* engine)
        : m_Engine(engine) {};

public:
    void InitializeSubsystems();
    void ShutdownSubsystems();

public:
    inline Window* GetWindow() const { return m_Window.get(); };
    inline GfxRenderer* GetRenderer() const { return m_Renderer.get(); };
    inline ImGuiManager* GetImGuiManager() const { return m_ImGuiManager.get(); };

private:
    void InitializeWindow();
    void InitializeRenderer();
    void InitializeImGui();

    void ShutdownWindow();
    void ShutdownRenderer();
    void ShutdownImGui();

private:
    std::unique_ptr<Window> m_Window;
    std::unique_ptr<GfxRenderer> m_Renderer;
    std::unique_ptr<ImGuiManager> m_ImGuiManager;

    Engine* m_Engine;
};
