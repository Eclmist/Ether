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

#include "ether.h"

Ether::Ether(const EngineConfig& config)
    : Application(config)
{
}

void Ether::Initialize()
{
    m_Engine->Initialize();
}

void Ether::LoadContent()
{

}

void Ether::UnloadContent()
{

}

void Ether::Destroy()
{

}

void Ether::OnUpdate(UpdateEventArgs& e)
{

}

void Ether::OnRender(RenderEventArgs& e)
{

}

void Ether::OnKeyPressed(KeyEventArgs& e)
{

}

void Ether::OnKeyReleased(KeyEventArgs& e)
{

}

void Ether::OnMouseButtonPressed(MouseEventArgs& e)
{

}

void Ether::OnMouseButtonReleased(MouseEventArgs& e)
{

}

void Ether::OnMouseMoved(MouseEventArgs& e)
{

}
