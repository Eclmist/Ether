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
#include <cmath>

using namespace Ether;

void SampleApp::Initialize()
{
    LogInfo("Initializing Application: Sample App");
    EngineCore::GetEngineConfig().SetClientWidth(1920);
    EngineCore::GetEngineConfig().SetClientHeight(1080);
    EngineCore::GetEngineConfig().SetClientName(L"Ether Sample App");

    m_CameraDistance = 10.0f;
    m_CameraRotation = { -0.4, 0.785398, 0 };
}

void SampleApp::LoadContent()
{
    Entity* debugCube = EngineCore::GetECSManager().CreateEntity("Debug Cube");

    debugCube->AddComponent<MeshComponent>();
    debugCube->AddComponent<VisualComponent>();
    debugCube->GetComponent<TransformComponent>()->SetPosition({ 0, 1, 0 });
    debugCube->GetComponent<TransformComponent>()->SetRotation({ 0, 0, 0 });

    Entity* debugCubeAnim = EngineCore::GetECSManager().CreateEntity("Debug Cube (Animated)");
    debugCubeAnim->AddComponent<MeshComponent>();
    debugCubeAnim->AddComponent<VisualComponent>();
    debugCubeAnim->GetComponent<TransformComponent>()->SetPosition({ 2.0, -4.0, 0 });
    debugCubeAnim->GetComponent<TransformComponent>()->SetRotation({ 0, 0, 0.23f });
    debugCubeAnim->GetComponent<TransformComponent>()->SetScale({ 1.20f, 1.40f, 1.23f });

    Entity* groundPlane = EngineCore::GetECSManager().CreateEntity("Ground Plane");
    groundPlane->AddComponent<MeshComponent>();
    groundPlane->AddComponent<VisualComponent>();
    groundPlane->GetComponent<TransformComponent>()->SetScale({ 1000, 0.01f, 1000 });

    for (int i = 0; i < 0; ++i)
    {
        Entity* newEntity = EngineCore::GetECSManager().CreateEntity("New Entity");
        newEntity->GetComponent<TransformComponent>()->SetPosition({ (float)(rand() % 100 - 50), (float)(rand() % 100 - 50), (float)(rand() % 100 - 50) });
        newEntity->GetComponent<TransformComponent>()->SetRotation({ (float)rand(), (float)rand(), (float)rand() });
        newEntity->AddComponent<MeshComponent>();
        newEntity->AddComponent<VisualComponent>();
    }
}

void SampleApp::UnloadContent()
{
}

void SampleApp::Shutdown()
{
}

void SampleApp::OnUpdate(const UpdateEventArgs& e)
{
    if (Input::GetKeyDown(Win32::KeyCode::KEYCODE_F3))
        EngineCore::GetEngineConfig().ToggleDebugGui();

    if (Input::GetKeyDown(Win32::KeyCode::KEYCODE_F11))
        EngineCore::GetMainWindow().SetFullscreen(!EngineCore::GetMainWindow().GetFullscreen());

    m_CameraDistance -= Input::GetMouseWheelDelta() / 120;
    m_CameraDistance = std::clamp(m_CameraDistance, 0.0f, 100.0f);

    if (Input::GetMouseButton(1))
    {
        m_CameraRotation.x -= Input::GetMouseDeltaY() / 500;
        m_CameraRotation.y -= Input::GetMouseDeltaX() / 500;

        constexpr float rad90 = DirectX::XMConvertToRadians(80);
        m_CameraRotation.x = std::clamp(m_CameraRotation.x, -rad90, rad90);
    }

    float x = sin((float)GetTimeSinceStart());
    EngineCore::GetECSManager().GetEntity(1)->GetComponent<TransformComponent>()->SetPosition({ x, 5, 5 });
}

void SampleApp::OnRender(const RenderEventArgs& e)
{
    const ethXMVector upDir = DirectX::XMVectorSet(0, 1, 0, 0);
    float aspectRatio = EngineCore::GetEngineConfig().GetClientWidth() / static_cast<float>(EngineCore::GetEngineConfig().GetClientHeight());

    ethXMMatrix xRot = DirectX::XMMatrixRotationX(m_CameraRotation.x);
    ethXMMatrix yRot = DirectX::XMMatrixRotationY(m_CameraRotation.y);

    ethXMMatrix viewMatrix = DirectX::XMMatrixMultiply(yRot, xRot);
    viewMatrix = DirectX::XMMatrixMultiply(viewMatrix, DirectX::XMMatrixTranslation(0, 0, m_CameraDistance));

    ethXMMatrix projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(80), aspectRatio, 0.1f, 1000.0f);

    e.m_GraphicContext->SetViewMatrix(viewMatrix);
    e.m_GraphicContext->SetProjectionMatrix(projectionMatrix);
}

