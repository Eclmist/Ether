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

#include <filesystem>
#include "toolmode/toolmain.h"
#include "toolmode/ipc/ipcmanager.h"
#include "engine/platform/win32/ethwin.h"
#include "engine/world/ecs/components/ecscameracomponent.h"
#include "engine/world/ecs/components/ecsvisualcomponent.h"
#include "engine/animation/animation.h"
#include "engine/animation/skeleton.h"
#include "asset/assetimporter.h"

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
    ResourceManager& resources = currentWorld.GetResourceManager();

    const std::string hdriPath = "D:\\Graphics_Projects\\Atelier\\Workspaces\\RtCamp11\\sky_26_2k.png";
    const std::string uiOverlayPath = "D:\\Graphics_Projects\\Atelier\\Workspaces\\RtCamp11\\UI\\camera_overlay.png";
    const std::string workspacePath = GetCommandLineOptions().GetWorkspacePath();
    const std::vector<std::string>& m_ImportPaths = GetCommandLineOptions().GetImportPaths();
    const std::vector<std::string>& m_FlatternedImportPaths = GetCommandLineOptions().GetFlatternedImportPaths();

    if (GetCommandLineOptions().HasImports())
    {
        const float meshScale = GetCommandLineOptions().GetImportScale();
        const std::string exportWorldName = GetCommandLineOptions().GetExportName();
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
            AssetImporter::Instance().Import(m_ImportPaths[i]);

        for (uint32_t i = 0; i < m_FlatternedImportPaths.size(); ++i)
            AssetImporter::Instance().Import(m_FlatternedImportPaths[i], true);

        AssetImporter::Instance().ImportTexture(hdriPath);
        AssetImporter::Instance().ImportTexture(uiOverlayPath);

        // Load from library files and serialize to world
        // This simulates user dragging resources from the editor resource browser into the scene,
        // then saving the world file.
        std::vector<std::unique_ptr<Graphics::StaticMesh>> staticMeshes;
        std::vector<std::unique_ptr<Graphics::SkinnedMesh>> skinnedMeshes;
        std::vector<AnimationClip*> animationClips;

        for (const auto& entry : std::filesystem::directory_iterator(libraryPath))
        {
            if (entry.path().extension().string() != ".eres")
                continue;

            IFileStream classIdStream(entry.path().string());
            std::string classID = Serializable::DeserializeClassID(classIdStream);

            static const StringID StaticMeshClassID = StringID(ETH_CLASS_ID_STATICMESH);
            static const StringID SkinnedMeshClassID = StringID(ETH_CLASS_ID_SKINNEDMESH);
            static const StringID SkeletonClassID = StringID(ETH_CLASS_ID_SKELETON);
            static const StringID AnimationClipClassID = StringID(ETH_CLASS_ID_ANIMATIONCLIP);
            static const StringID MaterialClassID = StringID(ETH_CLASS_ID_MATERIAL);
            static const StringID TextureClassID = StringID(ETH_CLASS_ID_TEXTURE);

            IFileStream assetFileStream(entry.path().string());
            if (classID == StaticMeshClassID)
            {
                staticMeshes.emplace_back(std::make_unique<Graphics::StaticMesh>());
                staticMeshes.back()->Deserialize(assetFileStream);
            }
            else if (classID == SkinnedMeshClassID)
            {
                skinnedMeshes.emplace_back(std::make_unique<Graphics::SkinnedMesh>());
                skinnedMeshes.back()->Deserialize(assetFileStream);
            }
            else if (classID == SkeletonClassID)
            {
                std::unique_ptr<Skeleton> skeleton = std::make_unique<Skeleton>();
                skeleton->Deserialize(assetFileStream);
                resources.RegisterSkeletonResource(std::move(skeleton));
            }
            else if (classID == AnimationClipClassID)
            {
                std::unique_ptr<AnimationClip> animationClip = std::make_unique<AnimationClip>();
                animationClips.push_back(animationClip.get());
                animationClip->Deserialize(assetFileStream);
                resources.RegisterAnimationClipResource(std::move(animationClip));
            }
            else if (classID == MaterialClassID)
            {
                std::unique_ptr<Graphics::Material> material = std::make_unique<Graphics::Material>();
                material->Deserialize(assetFileStream);
                resources.RegisterMaterialResource(std::move(material));
            }
            else if (classID == TextureClassID)
            {
                std::unique_ptr<Graphics::Texture> texture = std::make_unique<Graphics::Texture>();
                texture->Deserialize(assetFileStream);
                resources.RegisterTextureResource(std::move(texture));
            }
            else
            {
                LogWarning("Encountered unknown class ID during deserialization of engine types.");
            }
        }

        for (auto& staticMesh : staticMeshes)
        {
            Entity& entity = currentWorld.CreateEntity("Entity (" + staticMesh->GetGuid() + ")");
            entity.AddComponent<Ecs::EcsVisualComponent>();
            Ecs::EcsVisualComponent& visual = entity.GetComponent<Ecs::EcsVisualComponent>();
            visual.m_MeshGuid = staticMesh->GetGuid();
            visual.m_MaterialGuid = staticMesh->GetDefaultMaterialGuid();
            currentWorld.GetResourceManager().RegisterStaticMeshResource(std::move(staticMesh));
        }

        for (auto& skinnedMesh : skinnedMeshes)
        {
            Entity& entity = currentWorld.CreateEntity("Entity (" + skinnedMesh->GetGuid() + ")");
            entity.AddComponent<Ecs::EcsSkinnedVisualComponent>();
            Ecs::EcsSkinnedVisualComponent& visual = entity.GetComponent<Ecs::EcsSkinnedVisualComponent>();
            visual.m_MeshGuid = skinnedMesh->GetGuid();
            visual.m_MaterialGuid = skinnedMesh->GetDefaultMaterialGuid();
            visual.m_SkeletonGuid = skinnedMesh->GetSkeletonGuid();
            visual.m_AnimationGuid = skinnedMesh->GetAnimationGuid();
            resources.RegisterSkinnedMeshResource(std::move(skinnedMesh));
        }

        Entity& cameraObj = currentWorld.CreateCamera();
        cameraObj.GetComponent<Ecs::EcsCameraComponent>().SetHdriTextureID(AssetImporter::Instance().GetAssetGuid(hdriPath));
        cameraObj.GetComponent<Ecs::EcsCameraComponent>().m_UITextureID = AssetImporter::Instance().GetAssetGuid(uiOverlayPath);

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

void Ether::Toolmode::EtherHeadless::OnPreRender(const Ether::RenderEventArgs& e)
{
}

void Ether::Toolmode::EtherHeadless::OnPostRender()
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

    ethMatrix4x4 rotation = Transform::GetRotationMatrix(ethQuaternion::FromEuler(m_CameraTransform->m_Rotation));
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
