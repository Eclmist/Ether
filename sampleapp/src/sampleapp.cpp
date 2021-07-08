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
#include <algorithm> // for std::clamp

void SampleApp::Initialize()
{
    LogInfo("Initializing Sample App");
    Ether::g_EngineConfig.SetClientWidth(1920);
    Ether::g_EngineConfig.SetClientHeight(1080);
    Ether::g_EngineConfig.SetClientName(L"Ether Sample App");

    m_CameraDistance = 50.0f;
    m_CameraRotation = { 0, 0, 0 };
}

void SampleApp::LoadContent()
{
    m_DebugCube = new Ether::Entity(L"Debug Cube");
    Ether::MeshComponent* debugMeshComponent = new Ether::MeshComponent();
    m_DebugCube->AddComponent(debugMeshComponent);
    m_DebugCube->GetTransform()->SetTranslation({ -.0, -.0, 0 });
    m_DebugCube->GetTransform()->SetRotation({ 0, 0, 0 });
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
    if (Ether::Input::GetKeyDown(Ether::Win32::KeyCode::KEYCODE_F3))
        Ether::g_EngineConfig.ToggleDebugGui();

    m_CameraDistance -= Ether::Input::GetMouseWheelDelta() / 60;
    m_CameraDistance = std::clamp(m_CameraDistance, 0.0f, 100.0f);

    if (Ether::Input::GetMouseButton(1))
    {
        m_CameraRotation.x -= Ether::Input::GetMouseDeltaY() / 500;
        m_CameraRotation.y -= Ether::Input::GetMouseDeltaX() / 500;

        constexpr float rad90 = DirectX::XMConvertToRadians(80);
        m_CameraRotation.x = std::clamp(m_CameraRotation.x, -rad90, rad90);
    }
}

void SampleApp::OnRender(const Ether::RenderEventArgs& e)
{
    const Ether::ethXMVector upDir = DirectX::XMVectorSet(0, 1, 0, 0);
    float aspectRatio = Ether::g_EngineConfig.GetClientWidth() / static_cast<float>(Ether::g_EngineConfig.GetClientHeight());

    Ether::ethXMMatrix xRot = DirectX::XMMatrixRotationX(m_CameraRotation.x);
    Ether::ethXMMatrix yRot = DirectX::XMMatrixRotationY(m_CameraRotation.y);

    Ether::ethXMMatrix viewMatrix = DirectX::XMMatrixMultiply(yRot, xRot);
    viewMatrix = DirectX::XMMatrixMultiply(viewMatrix, DirectX::XMMatrixTranslation(0, 0, m_CameraDistance));

    Ether::ethXMMatrix projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(80), aspectRatio, 0.1f, 1000.0f);

    e.m_GraphicContext->SetViewMatrix(viewMatrix);
    e.m_GraphicContext->SetProjectionMatrix(projectionMatrix);
}

