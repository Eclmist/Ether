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

#include "engine.h"
#include "system/win32/window.h"
#include "graphic/gfx/gfxrenderer.h"

ETH_NAMESPACE_BEGIN

Engine::Engine()
{
}

void Engine::Initialize(const EngineConfig& engineConfig)
{
    LogInfo("Initializing Engine");
    LogError("Initializing Engine");
    LogGraphicsError("Initializing Engine");
    LogEngineFatal("Initializing Engine");
    LogEngineWarning("Initializing Engine");
    LogWin32Error("Initializing Engine");
    LogInfo("Initializing Engine");
    LogError("Initializing Engine");
    LogGraphicsError("Initializing Engine");
    LogEngineFatal("Initializing Engine");
    LogEngineWarning("Initializing Engine");
    LogWin32Error("Initializing Engine");
    LogInfo("Initializing Engine");
    LogError("Initializing Engine");
    LogGraphicsError("Initializing Engine");
    LogEngineFatal("Initializing Engine");
    LogEngineWarning("Initializing Engine");
    LogWin32Error("Initializing Engine");
    LogError("Initializing Engine");
    LogGraphicsError("Initializing Engine");
    LogEngineFatal("Initializing Engine");
    LogEngineWarning("Initializing Engine");
    LogWin32Error("Initializing Engine");
    LogInfo("Initializing Engine");
    LogError("Initializing Engine");
    LogGraphicsError("Initializing Engine");
    LogEngineFatal("Initializing Engine");
    LogEngineWarning("Initializing Engine");
    LogWin32Error("Initializing Engine");

    m_EngineConfig = engineConfig;

    InitializeSubsystems();
}

void Engine::LoadContent()
{

}

void Engine::UnloadContent()
{

}

void Engine::Shutdown()
{
    UnloadContent();
    ShutdownSubsystems();
}

void Engine::OnUpdate(UpdateEventArgs& e)
{
}

void Engine::OnRender(RenderEventArgs& e)
{
    g_GfxRenderer->RenderFrame();
}

void Engine::OnKeyPressed(KeyEventArgs& e)
{
    if (e.m_Key == KEYCODE_F3)
        g_GfxRenderer->ToggleGui();
}

void Engine::OnKeyReleased(KeyEventArgs& e)
{

}

void Engine::OnMouseButtonPressed(MouseEventArgs& e)
{

}

void Engine::OnMouseButtonReleased(MouseEventArgs& e)
{

}

void Engine::OnMouseMoved(MouseEventArgs& e)
{

}

void Engine::InitializeSubsystems()
{
    g_GfxRenderer = new GfxRenderer();

    g_GfxRenderer->Initialize();
}

void Engine::ShutdownSubsystems()
{
    delete g_GfxRenderer;
}

ETH_NAMESPACE_END

