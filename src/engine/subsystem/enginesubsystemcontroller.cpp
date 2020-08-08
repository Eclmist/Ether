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

#include "enginesubsystemcontroller.h"
#include "engine/engine.h"

void EngineSubsystemController::InitializeSubsystems()
{
    InitializeImGui();
    InitializeWindow();
    InitializeRenderer();
}

void EngineSubsystemController::ShutdownSubsystems()
{
    ShutdownRenderer();
    ShutdownWindow();
    ShutdownImGui();
}

void EngineSubsystemController::InitializeWindow()
{
    m_WindowManager = std::make_unique<WindowManager>(m_Engine);
    m_WindowManager->Initialize();
}

void EngineSubsystemController::InitializeRenderer()
{
    m_Renderer = std::make_unique<GfxRenderer>(m_Engine);
    m_Renderer->Initialize();
}

void EngineSubsystemController::InitializeImGui()
{
    m_ImGuiManager = std::make_unique<ImGuiManager>(m_Engine);
    m_ImGuiManager->Initialize();
}

void EngineSubsystemController::ShutdownWindow()
{
    m_WindowManager->Shutdown();
}

void EngineSubsystemController::ShutdownRenderer()
{
    m_Renderer->Shutdown();
}

void EngineSubsystemController::ShutdownImGui()
{
    m_ImGuiManager->Shutdown();
}

