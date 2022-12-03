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
#include "engine/world/ecs/component/ecsmeshcomponent.h"
//#include <common/stream/filestream.h>
//#include <filesystem>

using namespace Ether;

//std::string sceneRootPath = "D:\\Graphics_Projects\\Atelier\\Workspaces\\Sponza\\";
////std::string sceneRootPath = "D:\\Graphics_Projects\\Atelier\\Workspaces\\Bedroom\\";
//std::string objName = "sponza.obj.ether";
////std::string objName = "iscv2.obj.ether";
//std::string modelsGroupPath = sceneRootPath + objName;

void SampleApp::Initialize()
{
    LogInfo("Initializing Application: Sample App");

    //Ether::Client::SetClientSize({ 1366, 768 });
    Ether::Client::SetClientTitle("Ether Sample App");

    //m_CameraDistance = 10.0f;
    //m_CameraRotation = { -0.4, 0.785398, 0 };

    //EngineCore::GetEngineConfig().ToggleDebugGui();

}

void SampleApp::LoadContent()
{
    //for (int i = 0; i < 400; ++i)
    //{
    //    std::string modelPath = modelsGroupPath + std::to_string(i);
    //    if (!std::filesystem::exists(modelPath))
    //        break;

    //    std::shared_ptr<CompiledMesh> compiledMesh = std::make_shared<CompiledMesh>();
    //    compiledMesh->Deserialize(IFileStream(modelPath));


    //    Entity* entity = EngineCore::GetEcsManager().CreateEntity();
    //    entity->SetName(compiledMesh->GetName());

    //    MeshComponent* mesh = entity->AddComponent<MeshComponent>();
    //    mesh->SetCompiledMesh(compiledMesh);

    //    VisualComponent* visual = entity->AddComponent<VisualComponent>();
    //    visual->SetMaterial(compiledMesh->GetMaterial());

    //    TransformComponent* transform = entity->GetComponent<TransformComponent>();
    //    transform->SetPosition({ 0, -15, 0 });
    //    transform->SetRotation({ 0, 0, 0 });
    //    transform->SetScale({ 0.1 });

    //    LoadTexture(compiledMesh->GetMaterial()->m_AlbedoTexturePath, "_AlbedoTexture");
    //    if (m_Textures.find(compiledMesh->GetMaterial()->m_AlbedoTexturePath) != m_Textures.end())
    //        visual->GetMaterial()->SetTexture("_AlbedoTexture", m_Textures[compiledMesh->GetMaterial()->m_AlbedoTexturePath]);

    //    LoadTexture(compiledMesh->GetMaterial()->m_SpecularTexturePath, "_SpecularTexture");
    //    if (m_Textures.find(compiledMesh->GetMaterial()->m_SpecularTexturePath) != m_Textures.end())
    //        visual->GetMaterial()->SetTexture("_SpecularTexture", m_Textures[compiledMesh->GetMaterial()->m_SpecularTexturePath]);
    //}

    ////std::shared_ptr<CompiledTexture> hdri = std::make_shared<CompiledTexture>();
    ////IFileStream iistream("D:\\Graphics_Projects\\Atelier\\Workspaces\\Debug\\Hdri\\narrow_moonlit_road_4k.hdr.ether");
    ////hdri->Deserialize(iistream);
    ////GraphicCore::GetGraphicRenderer().m_EnvironmentHDRI = hdri;

    Ether::World& world = GetActiveWorld();
    auto& e0 = world.CreateEntity("Entity 0");
    auto& e1 = world.CreateEntity("Entity 1");
    auto& e2 = world.CreateEntity("Entity 2");
    auto& e3 = world.CreateEntity("Entity 3");
    auto& e4 = world.CreateEntity("Entity 4");
    auto& e5 = world.CreateEntity("Entity 5");

    e1.AddComponent<Ecs::EcsMeshComponent>();
}

void SampleApp::UnloadContent()
{
}

void SampleApp::Shutdown()
{
}

void SampleApp::OnUpdate(const UpdateEventArgs& e)
{
    //uint32_t y = (uint32_t)(200 * (sin(Time::GetCurrentTime() / 10000.0) + 2));

    //Ether::Client::SetClientSize({ 1920, 1080 });

    //if (Input::GetKeyDown(Win32::KeyCode::F3))
    //    EngineCore::GetEngineConfig().ToggleDebugGui();

    if (Input::GetKeyDown((KeyCode)Win32::KeyCode::F11))
        Ether::Client::SetFullscreen(!Ether::Client::IsFullscreen());

    //UpdateCamera(e.m_DeltaTime);
}

void SampleApp::UpdateCamera(float deltaTime)
{
    //m_ViewMatrix = ethMatrix4x4();
    //m_ViewMatrixInv = ethMatrix4x4();
    //m_ProjectionMatrix = ethMatrix4x4();
    //UpdateOrbitCam(deltaTime);
}

// TODO: Move into a camera component
ethMatrix4x4 GetPerspectiveMatrixLH(float fovy, float aspect, float znear, float zfar)
{
    const float range = tan(SMath::DegToRad(fovy / 2)) * znear;
    const float left = -range * aspect;
    const float right = range * aspect;
    const float bottom = -range;
    const float top = range;
    const float Q = zfar / (zfar - znear);

    ethMatrix4x4 dst(0.0f);
    dst.m_Data2D[0][0] = (2 * znear) / (right - left);
    dst.m_Data2D[1][1] = (2 * znear) / (top - bottom);
    dst.m_Data2D[2][2] = Q;
    dst.m_Data2D[2][3] = -znear * Q;
    dst.m_Data2D[3][2] = 1;
    return dst;
}

void SampleApp::UpdateOrbitCam(float deltaTime)
{
    //const float sensitivity = 0.15f;

    //m_CameraDistance -= Input::GetMouseWheelDelta() * deltaTime;
    //m_CameraDistance = std::clamp(m_CameraDistance, 0.0f, 100.0f);

    //if (Input::GetMouseButton(1))
    //{
    //    m_CameraRotation.x -= Input::GetMouseDeltaY() * sensitivity * deltaTime;
    //    m_CameraRotation.y -= Input::GetMouseDeltaX() * sensitivity * deltaTime;
    //    m_CameraRotation.x = ethClamp(m_CameraRotation.x, -ethDegToRad(90.0f), ethDegToRad(90.0f));
    //}

    //ethMatrix4x4 rotationMatrix = TransformComponent::GetRotationMatrixX(m_CameraRotation.x) * TransformComponent::GetRotationMatrixY(m_CameraRotation.y);
    //ethMatrix4x4 translationMatrix = TransformComponent::GetTranslationMatrix({ 0, 0, m_CameraDistance });

    //m_ViewMatrix = translationMatrix * rotationMatrix;
    //m_ViewMatrixInv = m_ViewMatrix.Inversed();

    //float aspectRatio = EngineCore::GetEngineConfig().GetClientWidth() / static_cast<float>(EngineCore::GetEngineConfig().GetClientHeight());
    //m_ProjectionMatrix = GetPerspectiveMatrixLH(80, aspectRatio, 0.01f, 1000.0f);
}

    void UpdateFlyCam(float deltaTime)
    {
        //if (Input::GetMouseButton(2))
        //{
        //    m_CameraRotation.x -= Input::GetMouseDeltaY() / 500;
        //    m_CameraRotation.y -= Input::GetMouseDeltaX() / 500;
        //    m_CameraRotation.x = std::clamp(m_CameraRotation.x, -DEG_TO_RAD(90), DEG_TO_RAD(90));
        //}

        //ethMatrix4x4 rotationMatrix = TransformComponent::GetRotationMatrixX(m_CameraRotation.x) * TransformComponent::GetRotationMatrixY(m_CameraRotation.y);
        //ethVector3 forwardVec = (rotationMatrix.Transposed() * {0, 0, 1}).Normalized();
        //ethVector3 upVec = XMVector4Normalize(g_XMIdentityR1);
        //ethVector3 rightVec = XMVector4Normalize(XMVector3Cross(upVec, forwardVec));

        //if (Input::GetMouseButton(2))
        //{
        //    if (Input::GetKey(Win32::KeyCode::W))
        //        m_CameraPosition = XMVectorSubtract(m_CameraPosition, forwardVec * deltaTime * 20);
        //    if (Input::GetKey(Win32::KeyCode::S))
        //        m_CameraPosition = XMVectorAdd(m_CameraPosition, forwardVec * deltaTime * 20);
        //    if (Input::GetKey(Win32::KeyCode::A))
        //        m_CameraPosition = XMVectorAdd(m_CameraPosition, rightVec * deltaTime * 20);
        //    if (Input::GetKey(Win32::KeyCode::D))
        //        m_CameraPosition = XMVectorSubtract(m_CameraPosition, rightVec * deltaTime * 20);
        //}

        //m_CameraPosition = XMVectorSubtract(m_CameraPosition, forwardVec * Input::GetMouseWheelDelta() / 121.0);

        //m_ViewMatrix = XMMatrixTranslationFromVector(m_CameraPosition) * rotationMatrix;
        //m_ViewMatrixInv = XMMatrixInverse(nullptr, m_ViewMatrix);

        //float aspectRatio = EngineCore::GetEngineConfig().GetClientWidth() / static_cast<float>(EngineCore::GetEngineConfig().GetClientHeight());
        //m_ProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(80), aspectRatio, 0.01f, 1000.0f);
    }


void SampleApp::LoadTexture(const std::string& path, const std::string& key)
{
    //if (path == "")
    //    return;

    //if (m_Textures.find(path) == m_Textures.end())
    //{
    //    std::shared_ptr<CompiledTexture> texture = std::make_shared<CompiledTexture>();
    //    IFileStream iistream(sceneRootPath + path + ".ether0");
    //    if (iistream.IsOpen())
    //    {
    //        texture->Deserialize(iistream);
    //        m_Textures[path] = texture;
    //    }
    //}

}

void SampleApp::OnRender(const RenderEventArgs& e)
{

}

void SampleApp::OnShutdown()
{

}

