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
#include "asset/assetimporter.h"
#include <filesystem>
#include "engine/world/ecs/components/ecsvisualcomponent.h"

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

// The idea of this block is to test toolmode functionality without having the actual tool developed yet
// For example:
//      - Asset import (menu > asset > import) (Not simulated because asset importer code are all in toolmode
//      sln)
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
void Ether::Toolmode::EtherHeadless::LoadContent()
{
    World& currentWorld = GetActiveWorld();

    const std::string hdriPath = "D:\\Graphics_Projects\\Atelier\\Workspaces\\Hdri\\kloofendal_48d_partly_cloudy_puresky_8k.hdr";
    const std::string workspacePath = GetCommandLineOptions().GetWorkspacePath();
    const std::vector<std::string>& m_ImportPaths = GetCommandLineOptions().GetImportPaths();
    const bool hasImports = !m_ImportPaths.empty();

    if (hasImports)
    {
        const float meshScale = GetCommandLineOptions().GetImportScale();
        const std::string exportWorldName = PathUtils::GetFileName(m_ImportPaths[0]);
        const std::string sceneSavePath = workspacePath + "\\" + exportWorldName + ".ether";
        const std::string libraryPath = workspacePath + "\\Library\\" + exportWorldName;
        AssetImporter::Instance().SetWorkspacePath(workspacePath);
        AssetImporter::Instance().SetLibraryPath(libraryPath);
        AssetImporter::Instance().SetMeshScale(meshScale);

        // Clean library directory
        std::filesystem::create_directory(libraryPath);
        for (const auto& entry : std::filesystem::directory_iterator(libraryPath))
        {
            if (entry.path().extension().string() != ".eres")
                continue;

            std::filesystem::remove(entry);
        }

        for (uint32_t i = 0; i < m_ImportPaths.size(); ++i)
            AssetImporter::Instance().ImportMesh(m_ImportPaths[i]);

        AssetImporter::Instance().ImportTexture(hdriPath);

        // Load from library files and serialize to world
        // This simulates user dragging resources from the editor resource browser into the scene,
        // then saving the world file.
        std::vector<std::unique_ptr<Graphics::Mesh>> meshes;
        for (const auto& entry : std::filesystem::directory_iterator(libraryPath))
        {
            if (entry.path().extension().string() != ".eres")
                continue;

            IFileStream classIdStream(entry.path().string());
            std::string classID = Serializable::DeserializeClassID(classIdStream);

            static const StringID MeshClassID = StringID(ETH_CLASS_ID_MESH);
            static const StringID MaterialClassID = StringID(ETH_CLASS_ID_MATERIAL);
            static const StringID TextureClassID = StringID(ETH_CLASS_ID_TEXTURE);

            if (classID == MeshClassID)
            {
                IFileStream assetFileStream(entry.path().string());
                meshes.emplace_back(std::make_unique<Graphics::Mesh>());
                meshes.back()->Deserialize(assetFileStream);
            }
            else if (classID == MaterialClassID)
            {
                IFileStream assetFileStream(entry.path().string());
                std::unique_ptr<Graphics::Material> material = std::make_unique<Graphics::Material>();
                material->Deserialize(assetFileStream);
                currentWorld.GetResourceManager().RegisterMaterialResource(std::move(material));
            }
            else if (classID == TextureClassID)
            {
                IFileStream assetFileStream(entry.path().string());
                std::unique_ptr<Graphics::Texture> texture = std::make_unique<Graphics::Texture>();
                texture->Deserialize(assetFileStream);
                currentWorld.GetResourceManager().RegisterTextureResource(std::move(texture));
            }
        }

        for (auto& mesh : meshes)
        {
            Entity& entity = currentWorld.CreateEntity("Entity (" + mesh->GetGuid() + ")");
            entity.AddComponent<Ecs::EcsVisualComponent>();
            Ecs::EcsVisualComponent& visual = entity.GetComponent<Ecs::EcsVisualComponent>();
            visual.m_MeshGuid = mesh->GetGuid();
            visual.m_MaterialGuid = mesh->GetDefaultMaterialGuid();
            currentWorld.GetResourceManager().RegisterMeshResource(std::move(mesh));
        }

        Entity& cameraObj = currentWorld.CreateCamera();
        cameraObj.GetComponent<Ecs::EcsCameraComponent>().SetHdriTextureID(AssetImporter::Instance().GetAssetGuid(hdriPath));

        currentWorld.SetWorldName(exportWorldName);
        currentWorld.Save(sceneSavePath);
        PostQuitMessage(0);
    }

    const std::string importWorldName = GetCommandLineOptions().GetWorldName();
    const std::string sceneLoadPath = workspacePath + "\\" + importWorldName;

    if (PathUtils::GetFileExtension(sceneLoadPath) == ".ether")
        currentWorld.Load(sceneLoadPath);

    Entity& cameraObj = currentWorld.CreateCamera();
    m_CameraTransform = &cameraObj.GetComponent<Ecs::EcsTransformComponent>();
    m_CameraTransform->m_Translation = { 0, 2, 0 };
    m_CameraTransform->m_Rotation = { 0, SMath::DegToRad(-90.0f), 0 };
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

    UpdateGraphicConfig();
    UpdateCamera();
}

void Ether::Toolmode::EtherHeadless::OnRender(const Ether::RenderEventArgs& e)
{
}

void Ether::Toolmode::EtherHeadless::OnShutdown()
{
}

void Ether::Toolmode::EtherHeadless::UpdateGraphicConfig() const
{
    Ether::Graphics::GraphicConfig& graphicConfig = Ether::Graphics::GetGraphicConfig();

    if (Input::GetKeyDown((KeyCode)Win32::KeyCode::F11))
        Ether::Client::SetFullscreen(!Ether::Client::IsFullscreen());

    if (Input::GetKeyDown((KeyCode)Win32::KeyCode::Space))
        graphicConfig.m_IsRaytracingDebugEnabled = !graphicConfig.m_IsRaytracingDebugEnabled;

    if (Input::GetKeyDown((KeyCode)Win32::KeyCode::F3))
        graphicConfig.SetDebugGuiEnabled(!graphicConfig.IsDebugGuiEnabled());

    if (Input::GetKey((KeyCode)Win32::KeyCode::J))
        graphicConfig.m_SunDirection = (graphicConfig.m_SunDirection +
                                        Ether::ethVector4(-1, 0, 0, 0) * Time::GetDeltaTime() * 0.001)
                                           .Normalized();
    if (Input::GetKey((KeyCode)Win32::KeyCode::L))
        graphicConfig.m_SunDirection = (graphicConfig.m_SunDirection +
                                        Ether::ethVector4(1, 0, 0, 0) * Time::GetDeltaTime() * 0.001)
                                           .Normalized();
    if (Input::GetKey((KeyCode)Win32::KeyCode::I))
        graphicConfig.m_SunDirection = (graphicConfig.m_SunDirection +
                                        Ether::ethVector4(0, 0, 1, 0) * Time::GetDeltaTime() * 0.001)
                                           .Normalized();
    if (Input::GetKey((KeyCode)Win32::KeyCode::K))
        graphicConfig.m_SunDirection = (graphicConfig.m_SunDirection +
                                        Ether::ethVector4(0, 0, -1, 0) * Time::GetDeltaTime() * 0.001)
                                           .Normalized();
    if (Input::GetKey((KeyCode)Win32::KeyCode::U))
        graphicConfig.m_SunDirection =
            (graphicConfig.m_SunDirection + Ether::ethVector4(0, 1, 0, 0) * Time::GetDeltaTime());
    if (Input::GetKey((KeyCode)Win32::KeyCode::O))
        graphicConfig.m_SunDirection =
            (graphicConfig.m_SunDirection + Ether::ethVector4(0, -1, 0, 0) * Time::GetDeltaTime());
}

void Ether::Toolmode::EtherHeadless::UpdateCamera() const
{
    static ethVector3 cameraRotation;
    static float moveSpeed = 1.0f;

    if (Input::GetKey((KeyCode)Win32::KeyCode::ShiftKey))
        moveSpeed = 2.0f;
    else
        moveSpeed = 1.0f;

    if (Input::GetMouseButton(2))
    {
        m_CameraTransform->m_Rotation.x += Input::GetMouseDeltaY() / 500;
        m_CameraTransform->m_Rotation.y += Input::GetMouseDeltaX() / 500;
        m_CameraTransform->m_Rotation.x = std::clamp(
            m_CameraTransform->m_Rotation.x,
            -SMath::DegToRad(89.0f),
            SMath::DegToRad(89.0f));
    }

    if (Input::GetKey((KeyCode)Win32::KeyCode::E))
        m_CameraTransform->m_Translation.y += Time::GetDeltaTime() * moveSpeed;

    if (Input::GetKey((KeyCode)Win32::KeyCode::Q))
        m_CameraTransform->m_Translation.y -= Time::GetDeltaTime() * moveSpeed;

    ethMatrix4x4 rotation = Transform::GetRotationMatrix(m_CameraTransform->m_Rotation);
    ethVector3 forward = (rotation * ethVector4(0, 0, 1, 0)).Resize<3>().Normalized();
    ethVector3 upVec = { 0, 1, 0 };
    ethVector3 rightVec = ethVector3::Cross(upVec, forward).Normalized();

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
