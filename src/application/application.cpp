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

#include "application.h"

Application::Application(const EngineConfig& config)
{
    m_Engine = std::make_unique<Engine>(config);
}

void Application::Initialize()
{
}

void Application::Destroy()
{
}

void Application::OnUpdate(UpdateEventArgs& e)
{

}

void Application::OnRender(RenderEventArgs& e)
{

}

void Application::OnKeyPressed(KeyEventArgs& e)
{

}

void Application::OnKeyReleased(KeyEventArgs& e)
{

}

void Application::OnMouseButtonPressed(MouseEventArgs& e)
{

}

void Application::OnMouseButtonReleased(MouseEventArgs& e)
{

}

void Application::OnMouseMoved(MouseEventArgs& e)
{

}
