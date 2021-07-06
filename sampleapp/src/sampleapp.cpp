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

#include "sampleapp.h"

void SampleApp::Initialize()
{
    LogInfo("Initializing Sample App");
    Ether::g_EngineConfig.SetClientWidth(1366);
    Ether::g_EngineConfig.SetClientHeight(768);
    Ether::g_EngineConfig.SetClientName(L"Ether Sample App");
}

void SampleApp::LoadContent()
{
    m_DebugCube = new Ether::Entity(L"Debug Cube");
    Ether::MeshComponent* debugMeshComponent = new Ether::MeshComponent();
    m_DebugCube->AddComponent(debugMeshComponent);
    Ether::g_World.AddEntity(m_DebugCube);
}

void SampleApp::UnloadContent()
{
    delete m_DebugCube;
}

void SampleApp::Shutdown()
{
}

void SampleApp::OnUpdate(const Ether::UpdateEventArgs& e)
{
}

void SampleApp::OnRender(const Ether::RenderEventArgs& e)
{
}

void SampleApp::OnKeyPress(const Ether::KeyEventArgs& e)
{
    if (e.m_Key == Ether::Win32::KEYCODE_F3)
        Ether::g_EngineConfig.ToggleDebugGui();
}

void SampleApp::OnKeyRelease(const Ether::KeyEventArgs& e)
{
}

void SampleApp::OnMouseButtonPress(const Ether::MouseEventArgs& e)
{
}

void SampleApp::OnMouseButtonRelease(const Ether::MouseEventArgs& e)
{
}

void SampleApp::OnMouseMove(const Ether::MouseEventArgs& e)
{
}
