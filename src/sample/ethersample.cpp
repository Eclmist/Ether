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

#include "ethersample.h"

EtherSample::EtherSample(const EngineConfig& config)
    : Engine(config)
{
}

void EtherSample::Initialize()
{
    Engine::Initialize();
}

void EtherSample::LoadContent()
{
    // TODO : Load demo scene
}

void EtherSample::UnloadContent()
{
    // TODO : Unload demo scene
}

void EtherSample::Shutdown()
{
    Engine::Shutdown();
}

void EtherSample::OnUpdate(UpdateEventArgs& e)
{
    Engine::OnUpdate(e);
}

void EtherSample::OnRender(RenderEventArgs& e)
{
    Engine::OnRender(e);
}

void EtherSample::OnKeyPressed(KeyEventArgs& e)
{
    Engine::OnKeyPressed(e);

    switch (e.m_Key)
    {
    case KEYCODE_F3:
        GetRenderer()->ToggleGui();
        break;
    case KEYCODE_F11:
        GetWindow()->ToggleFullscreen();
        break;
    default:
        break;
    }
}

void EtherSample::OnKeyReleased(KeyEventArgs& e)
{
    Engine::OnKeyReleased(e);
}

void EtherSample::OnMouseButtonPressed(MouseEventArgs& e)
{
    Engine::OnMouseButtonPressed(e);
}

void EtherSample::OnMouseButtonReleased(MouseEventArgs& e)
{
    Engine::OnMouseButtonReleased(e);
}

void EtherSample::OnMouseMoved(MouseEventArgs& e)
{
    Engine::OnMouseMoved(e);
}

void EtherSample::OnEditorViewportResize(uint32_t width, uint32_t height)
{
    GetWindow()->SetViewportRect({0, 0, (LONG)width, (LONG)height});
}

