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

#include "sampleapp.h"
#include "engine/world/ecs/components/ecsvisualcomponent.h"
#include "engine/world/ecs/components/ecscameracomponent.h"
#include <format>
#include <algorithm>
#include <filesystem>

using namespace Ether;

void SampleApp::Initialize()
{
    LogInfo("Initializing Application: Sample App");
    Client::SetClientTitle("Ether Sample App");
    Client::SetClientSize({ 1920, 1080 });
}

void SampleApp::LoadContent()
{
    World& world = GetActiveWorld();

#if 0
    // The idea of this block is to test toolmode functionality without having the actual tool developed yet
    // For example:
    //      - Asset import (menu > asset > import) (Not simulated because asset importer code are all in toolmode sln)
    //          this generates a library of .eres files. In practice, toolmode itself should serialize this library
    //          which could contain guid to type mappings, and could reload all the guids during toolmode runtime.
    //      - Build resource table
    //          this is simulated by blindly loading all .eres files and assuming them to be meshes
    //      - Create entity (menu > new > entity)
    //          simulated by creating entity object
    //      - Assign mesh to entity (through components)
    //          simulated by AddComponent<Visual> and assigning mesh guid
    //      - Save scene
    //          World.save();

    std::unique_ptr<Graphics::Material> sharedMaterial = std::make_unique<Graphics::Material>();
    sharedMaterial->SetBaseColor({ 1, 1, 1, 1 });
    sharedMaterial->SetSpecularColor({ 1, 1, 1, 1 });
    sharedMaterial->SetMetalness(0);
    sharedMaterial->SetRoughness(0.5f);

    std::string path = "D:\\Graphics_Projects\\Atelier\\Workspaces\\Debug\\";
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        if (entry.path().extension().string() != ".eres")
            continue;

        std::unique_ptr<Graphics::Mesh> mesh = std::make_unique<Graphics::Mesh>();

        IFileStream ifstream(entry.path().string());
        mesh->Deserialize(ifstream);
        mesh->CreateGpuResources();

        Entity& entity = world.CreateEntity("Entity");
        entity.AddComponent<Ecs::EcsVisualComponent>();

        Ecs::EcsVisualComponent& visual = entity.GetComponent<Ecs::EcsVisualComponent>();
        visual.m_MeshGuid = mesh->GetGuid();
        visual.m_MaterialGuid = sharedMaterial->GetGuid();

        world.GetResourceManager().RegisterMeshResource(std::move(mesh));
    }

    world.GetResourceManager().RegisterMaterialResource(std::move(sharedMaterial));

    world.Save("D:\\Graphics_Projects\\Atelier\\Workspaces\\Debug\\TestScene.ether");
#else
    world.Load("D:\\Graphics_Projects\\Atelier\\Workspaces\\Debug\\TestScene.ether");

#endif

    Entity& camera = world.CreateEntity("Main Camera");
    camera.AddComponent<Ecs::EcsCameraComponent>();
    m_CameraTransform = &camera.GetComponent<Ecs::EcsTransformComponent>();
}

void SampleApp::UnloadContent()
{
}

void SampleApp::Shutdown()
{
}

void SampleApp::OnUpdate(const UpdateEventArgs& e)
{
    // uint32_t y = (uint32_t)(200 * (sin(Time::GetCurrentTime() / 10000.0) + 2));

    // Ether::Client::SetClientSize({ 1920, 1080 });

    // if (Input::GetKeyDown(Win32::KeyCode::F3))
    //     EngineCore::GetEngineConfig().ToggleDebugGui();
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
        m_CameraTransform->m_Rotation.x = std::clamp(
            (double)m_CameraTransform->m_Rotation.x,
            -SMath::DegToRad(90),
            SMath::DegToRad(90));
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
        m_CameraTransform->m_Translation = m_CameraTransform->m_Translation +
                                           forward * Time::GetDeltaTime() * moveSpeed;
    if (Input::GetKey((KeyCode)Win32::KeyCode::A))
        m_CameraTransform->m_Translation = m_CameraTransform->m_Translation -
                                           rightVec * Time::GetDeltaTime() * moveSpeed;
    if (Input::GetKey((KeyCode)Win32::KeyCode::S))
        m_CameraTransform->m_Translation = m_CameraTransform->m_Translation -
                                           forward * Time::GetDeltaTime() * moveSpeed;
    if (Input::GetKey((KeyCode)Win32::KeyCode::D))
        m_CameraTransform->m_Translation = m_CameraTransform->m_Translation +
                                           rightVec * Time::GetDeltaTime() * moveSpeed;
}

void SampleApp::LoadTexture(const std::string& path, const std::string& key)
{
    // if (path == "")
    //     return;

    // if (m_Textures.find(path) == m_Textures.end())
    //{
    //     std::shared_ptr<CompiledTexture> texture = std::make_shared<CompiledTexture>();
    //     IFileStream iistream(sceneRootPath + path + ".ether0");
    //     if (iistream.IsOpen())
    //     {
    //         texture->Deserialize(iistream);
    //         m_Textures[path] = texture;
    //     }
    // }
}

void SampleApp::OnRender(const RenderEventArgs& e)
{
}

void SampleApp::OnShutdown()
{
}
