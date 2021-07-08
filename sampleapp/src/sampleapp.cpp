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
    Ether::g_EngineConfig.SetClientWidth(1920);
    Ether::g_EngineConfig.SetClientHeight(1080);
    Ether::g_EngineConfig.SetClientName(L"Ether Sample App");

    m_CameraPos = { 0, 0, -10 };
    m_CameraDir = { 0, 0, 1 };
}

void SampleApp::LoadContent()
{
    m_DebugCube = new Ether::Entity(L"Debug Cube");
    Ether::MeshComponent* debugMeshComponent = new Ether::MeshComponent();
    m_DebugCube->AddComponent(debugMeshComponent);
    m_DebugCube->GetTransform()->SetTranslation({ -.0, -.0, 50 });
    m_DebugCube->GetTransform()->SetRotation({ 
        DirectX::XMConvertToRadians(45), DirectX::XMConvertToRadians(45), 0 });
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
    const Ether::ethXMVector eyePos = DirectX::XMLoadFloat3(&m_CameraPos);
    const Ether::ethXMVector eyeDir = DirectX::XMLoadFloat3(&m_CameraDir);
    const Ether::ethXMVector upDir = DirectX::XMVectorSet(0, 1, 0, 0);
    float aspectRatio = Ether::g_EngineConfig.GetClientWidth() / static_cast<float>(Ether::g_EngineConfig.GetClientHeight());

    Ether::ethXMMatrix viewMatrix = DirectX::XMMatrixLookToLH(eyePos, eyeDir, upDir);
    Ether::ethXMMatrix projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(80), aspectRatio, 0.1f, 100.0f);

    e.m_GraphicContext->SetViewMatrix(viewMatrix);
    e.m_GraphicContext->SetProjectionMatrix(projectionMatrix);
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
