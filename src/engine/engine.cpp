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

#include "engine.h"
#include "system/win32/windowmanager.h"

Engine::Engine(const EngineConfig& engineConfig)
    : m_EngineConfig(engineConfig)
    , m_SubsystemController(this)
{
}

void Engine::Initialize()
{
    m_SubsystemController.InitializeSubsystems();
}

void Engine::Shutdown()
{
    UnloadContent();
    m_SubsystemController.ShutdownSubsystems();
}

void Engine::OnUpdate(UpdateEventArgs& e)
{

}

void Engine::OnRender(RenderEventArgs& e)
{
    GetRenderer()->RenderFrame();
}

void Engine::OnKeyPressed(KeyEventArgs& e)
{

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

