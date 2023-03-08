/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2023 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "toolmode/toolmain.h"
#include "toolmode/ipc/ipcmanager.h"
#include "engine/platform/win32/ethwin.h"
#include "engine/world/ecs/components/ecscameracomponent.h"

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int cmdShow)
{
    CreateMutex(0, false, "Local\\Ether.Toolmode");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        MessageBox(nullptr, "An instance of Ether is already running.", "Ether", MB_OK | MB_ICONEXCLAMATION);
        return EXIT_FAILURE;
    }

    Ether::Toolmode::EtherHeadless etherHeadless;
    return Start(etherHeadless);
}


void Ether::Toolmode::EtherHeadless::Initialize()
{
}

void Ether::Toolmode::EtherHeadless::LoadContent()
{
    World& world = GetActiveWorld();
    world.Load("D:\\Graphics_Projects\\Atelier\\Workspaces\\Debug\\TestScene.ether");

    Entity& camera = world.CreateEntity("Main Camera");
    camera.AddComponent<Ecs::EcsCameraComponent>();
    m_CameraTransform = &camera.GetComponent<Ecs::EcsTransformComponent>();
}

void Ether::Toolmode::EtherHeadless::UnloadContent()
{
}

void Ether::Toolmode::EtherHeadless::Shutdown()
{
}

void Ether::Toolmode::EtherHeadless::OnUpdate(const Ether::UpdateEventArgs& e)
{
    IpcManager::Instance().ProcessIncomingCommands();
    IpcManager::Instance().ProcessOutgoingCommands();

    static ethVector3 cameraRotation;
    static float moveSpeed = 0.001f;

    if (Input::GetKey((KeyCode)Win32::KeyCode::ShiftKey))
        moveSpeed = 0.002f;
    else
        moveSpeed = 0.001f;

    if (Input::GetMouseButton(2))
    {
        m_CameraTransform->m_Rotation.x += Input::GetMouseDeltaY() / 500;
        m_CameraTransform->m_Rotation.y += Input::GetMouseDeltaX() / 500;
        m_CameraTransform->m_Rotation.x = std::clamp((double)m_CameraTransform->m_Rotation.x, -SMath::DegToRad(90), SMath::DegToRad(90));
    }

    if (Input::GetKeyDown((KeyCode)Win32::KeyCode::F11))
        Ether::Client::SetFullscreen(!Ether::Client::IsFullscreen());

    if (Input::GetKey((KeyCode)Win32::KeyCode::E))
        m_CameraTransform->m_Translation.y += Time::GetDeltaTime() * moveSpeed;

    if (Input::GetKey((KeyCode)Win32::KeyCode::Q))
        m_CameraTransform->m_Translation.y -= Time::GetDeltaTime() * moveSpeed;


    ethMatrix4x4 rotation = Transform::GetRotationMatrix(m_CameraTransform->m_Rotation);
    ethVector3 forward = (rotation * ethVector4(0, 0, 1, 0)).Resize<3>().Normalized();
    ethVector3 upVec = { 0, 1, 0 };
    ethVector3 rightVec = ethVector3::Cross(upVec, forward);

    if (Input::GetKey((KeyCode)Win32::KeyCode::W))
        m_CameraTransform->m_Translation = m_CameraTransform->m_Translation + forward * Time::GetDeltaTime() * moveSpeed;
    if (Input::GetKey((KeyCode)Win32::KeyCode::A))
        m_CameraTransform->m_Translation = m_CameraTransform->m_Translation - rightVec * Time::GetDeltaTime() * moveSpeed;
    if (Input::GetKey((KeyCode)Win32::KeyCode::S))
        m_CameraTransform->m_Translation = m_CameraTransform->m_Translation - forward * Time::GetDeltaTime() * moveSpeed;
    if (Input::GetKey((KeyCode)Win32::KeyCode::D))
        m_CameraTransform->m_Translation = m_CameraTransform->m_Translation + rightVec * Time::GetDeltaTime() * moveSpeed;
}

void Ether::Toolmode::EtherHeadless::OnRender(const Ether::RenderEventArgs& e)
{
}

void Ether::Toolmode::EtherHeadless::OnShutdown()
{
}
