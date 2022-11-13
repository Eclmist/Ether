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

#include <string>
#include "api/api.h"
#include <common/stream/filestream.h>
#include "toolmode/utility/pathutils.h"

ETH_NAMESPACE_BEGIN

std::string sceneRootPath = "D:\\Graphics_Projects\\Atelier\\Workspaces\\Sponza\\";
std::string objName = "sponza.obj.ether";
std::string modelsGroupPath = sceneRootPath + objName;

class EtherToolmode : public IApplicationBase
{
public:
    void Initialize() override
    {
        LogInfo("Initializing Application: Ether Toolmode");

        EngineCore::GetEngineConfig().SetClientWidth(1920);
        EngineCore::GetEngineConfig().SetClientHeight(1080);
        EngineCore::GetEngineConfig().SetClientName(L"Ether Toolmode");

        m_CameraMode = CameraMode::Orbit;
        m_CameraDistance = 10.0f;
        m_CameraRotation = { -0.4, 0.785398, 0 };
        m_OrthoX = 0;
        m_OrthoZ = 0;
        EngineCore::GetEngineConfig().ToggleDebugGui();
    };

    void LoadContent() override
    {
        for (int i = 0; i < 400; ++i)
        {
            std::string modelPath = modelsGroupPath + std::to_string(i);
            if (!PathUtils::IsValidPath(modelPath))
                break;

            std::shared_ptr<CompiledMesh> compiledMesh = std::make_shared<CompiledMesh>();
            compiledMesh->Deserialize(IFileStream(modelPath));


            Entity* entity = EngineCore::GetEcsManager().CreateEntity();
            entity->SetName(compiledMesh->GetName());

            MeshComponent* mesh = entity->AddComponent<MeshComponent>();
            mesh->SetCompiledMesh(compiledMesh);

            VisualComponent* visual = entity->AddComponent<VisualComponent>();
            visual->SetMaterial(compiledMesh->GetMaterial());

            TransformComponent* transform = entity->GetComponent<TransformComponent>();
            transform->SetPosition({ 0, -15, 0 });
            transform->SetRotation({ 0, 0, 0 });
            transform->SetScale({ 0.1 });

            LoadTexture(compiledMesh->GetMaterial()->m_AlbedoTexturePath, "_AlbedoTexture");
            if (m_Textures.find(compiledMesh->GetMaterial()->m_AlbedoTexturePath) != m_Textures.end())
                visual->GetMaterial()->SetTexture("_AlbedoTexture", m_Textures[compiledMesh->GetMaterial()->m_AlbedoTexturePath]);

            LoadTexture(compiledMesh->GetMaterial()->m_SpecularTexturePath, "_SpecularTexture");
            if (m_Textures.find(compiledMesh->GetMaterial()->m_SpecularTexturePath) != m_Textures.end())
                visual->GetMaterial()->SetTexture("_SpecularTexture", m_Textures[compiledMesh->GetMaterial()->m_SpecularTexturePath]);
        }

        //std::shared_ptr<CompiledTexture> hdri = std::make_shared<CompiledTexture>();
        //IFileStream iistream("D:\\Graphics_Projects\\Atelier\\Workspaces\\Debug\\Hdri\\narrow_moonlit_road_4k.hdr.ether");
        //hdri->Deserialize(iistream);
        //GraphicCore::GetGraphicRenderer().m_EnvironmentHDRI = hdri;
    };

    void LoadTexture(const std::string& path, const std::string& key)
    {
        if (path == "")
            return;

        if (m_Textures.find(path) == m_Textures.end())
        {
            std::shared_ptr<CompiledTexture> texture = std::make_shared<CompiledTexture>();
            IFileStream iistream(sceneRootPath + path + ".ether0");
            if (iistream.IsOpen())
            {
                texture->Deserialize(iistream);
                m_Textures[path] = texture;
            }
        }
    }

    void UnloadContent() override {};
    void Shutdown() override {};

public:
    void OnUpdate(const UpdateEventArgs& e) override
    {

        if (Input::GetKeyDown(Win32::KeyCode::F2))
            m_CameraMode = (CameraMode)(((uint32_t)m_CameraMode + 1) % (uint32_t)CameraMode::NumModes);

        if (Input::GetKeyDown(Win32::KeyCode::F3))
            EngineCore::GetEngineConfig().ToggleDebugGui();

        if (Input::GetKeyDown(Win32::KeyCode::F11))
            EngineCore::GetMainWindow().SetFullscreen(!EngineCore::GetMainWindow().GetFullscreen());

        UpdateCamera(e.m_DeltaTime);
    };

    void OnRender(const RenderEventArgs& e) override
    {
        const ethVector3 upDir = { 0, 1, 0 };

        e.m_GraphicContext->SetViewMatrix(m_ViewMatrix);
        e.m_GraphicContext->SetProjectionMatrix(m_ProjectionMatrix);
        e.m_GraphicContext->SetEyeDirection({ m_ViewMatrix.m_31, m_ViewMatrix.m_32, m_ViewMatrix.m_33, 1.0 });
        e.m_GraphicContext->SetEyePosition({ m_ViewMatrixInv.m_14, m_ViewMatrixInv.m_24, m_ViewMatrixInv.m_34, 0.0 });
    };

private:
    void UpdateCamera(float deltaTime)
    {
        ResetMatrices();

        switch (m_CameraMode)
        {
        case CameraMode::Orbit:
            return UpdateOrbitCam(deltaTime);
        case CameraMode::Fly:
            return UpdateFlyCam(deltaTime);
        case CameraMode::Ortho:
            return UpdateOrthoCam(deltaTime);
        }
    }

    void UpdateOrbitCam(float deltaTime)
    {
        m_CameraDistance -= Input::GetMouseWheelDelta() / 121;
        m_CameraDistance = std::clamp(m_CameraDistance, 0.0f, 100.0f);

        if (Input::GetMouseButton(1))
        {
            m_CameraRotation.x -= Input::GetMouseDeltaY() / 500;
            m_CameraRotation.y -= Input::GetMouseDeltaX() / 500;
            m_CameraRotation.x = ethClamp(m_CameraRotation.x, -ethDegToRad(90.0f), ethDegToRad(90.0f));
        }

        ethMatrix4x4 rotationMatrix = TransformComponent::GetRotationMatrixX(m_CameraRotation.x) * TransformComponent::GetRotationMatrixY(m_CameraRotation.y);
        ethMatrix4x4 translationMatrix = TransformComponent::GetTranslationMatrix({ 0, 0, m_CameraDistance });

        m_ViewMatrix = translationMatrix * rotationMatrix;
        m_ViewMatrixInv = m_ViewMatrix.Inversed();

        float aspectRatio = EngineCore::GetEngineConfig().GetClientWidth() / static_cast<float>(EngineCore::GetEngineConfig().GetClientHeight());
        m_ProjectionMatrix = GetPerspectiveMatrixLH(80, aspectRatio, 0.01f, 1000.0f);
    }

    // TODO: Move into a camera component
    ethMatrix4x4 GetPerspectiveMatrixRH(float fovy, float aspect, float znear, float zfar)
    {
        const float range = tan(ethDegToRad(fovy / 2)) * znear;
        const float left = -range * aspect;
        const float right = range * aspect;
        const float bottom = -range;
        const float top = range;
        const float Q = zfar / (znear - zfar);

        ethMatrix4x4 dst(0.0f);
        dst.m_Data2D[0][0] = (2 * znear) / (right - left);
        dst.m_Data2D[1][1] = (2 * znear) / (top - bottom);
        dst.m_Data2D[2][2] = Q;
        dst.m_Data2D[2][3] = -1;
        dst.m_Data2D[3][2] = znear * Q;
        return dst;
    }

    ethMatrix4x4 GetPerspectiveMatrixLH(float fovy, float aspect, float znear, float zfar)
    {
        const float range = tan(ethDegToRad(fovy / 2)) * znear;
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

    ethMatrix4x4 GetOrthographicMatrixLH(float w, float h, float n, float f)
    {
        return {
            2.0f / w, 0.0f, 0.0f, 0.0f,
            0.0f, 2.0f / h, 0.0f, 0.0f,
            0.0f, 0.0f, -2.0f / (f - n), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
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

    void UpdateOrthoCam(float deltaTime)
    {
        const float scaleModifier = 0.001;
        const float zoomModifier = 1.0 / 121.0;

        if (Input::GetMouseButtonDown(1))
            m_DragStartPos = { (float)Input::GetMousePosX(), (float)Input::GetMousePosY() };

        if (Input::GetMouseButton(1))
        {
            m_OrthoX += (Input::GetMousePosX() - m_DragStartPos.x) * m_CameraDistance * scaleModifier;
            m_OrthoZ -= (Input::GetMousePosY() - m_DragStartPos.y) * m_CameraDistance * scaleModifier;
            m_DragStartPos = { (float)Input::GetMousePosX(), (float)Input::GetMousePosY() };
        }

        if (abs(Input::GetMouseWheelDelta()) > 0)
        {
            float cameraDistanceDelta = Input::GetMouseWheelDelta() * zoomModifier;
            float offsetModifier = scaleModifier * cameraDistanceDelta;

            m_OrthoX += ((float)EngineCore::GetEngineConfig().GetClientWidth() / 2.0 - Input::GetMousePosX()) * offsetModifier;
            m_OrthoZ -= ((float)EngineCore::GetEngineConfig().GetClientHeight() / 2.0 - Input::GetMousePosY()) * offsetModifier;
            m_CameraDistance -= cameraDistanceDelta;
        }

        ethMatrix4x4 rotationMatrix = TransformComponent::GetRotationMatrixX(-90 * 0.0174533);
        ethMatrix4x4 translationMatrix = TransformComponent::GetTranslationMatrix({ m_OrthoX, m_OrthoZ, m_CameraDistance });
        m_ViewMatrix = rotationMatrix * translationMatrix;
        m_ViewMatrixInv = m_ViewMatrix.Inversed();
        m_ProjectionMatrix = GetOrthographicMatrixLH(
            (float)EngineCore::GetEngineConfig().GetClientWidth() * m_CameraDistance * scaleModifier,
            (float)EngineCore::GetEngineConfig().GetClientHeight() * m_CameraDistance * scaleModifier,
            0.01f, 1000.0f);
    }

    void ResetMatrices()
    {
        m_ViewMatrix = ethMatrix4x4();
        m_ViewMatrixInv = ethMatrix4x4();
        m_ProjectionMatrix = ethMatrix4x4();
    }

private:
    enum class CameraMode : unsigned int
    {
        Orbit,
        Fly,
        Ortho,
        NumModes
    };

    CameraMode m_CameraMode;

    // Orbit Cam
    float m_CameraDistance;
    ethVector3 m_CameraRotation;

    // Fly Cam
    ethVector3 m_CameraPosition;

    // Ortho Cam
    ethVector2 m_DragStartPos;
    float m_OrthoX, m_OrthoZ;

    ethMatrix4x4 m_ViewMatrix;
    ethMatrix4x4 m_ViewMatrixInv;
    ethMatrix4x4 m_ProjectionMatrix;

    std::shared_ptr<Material> m_Material;
    std::unordered_map<std::string, std::shared_ptr<Ether::CompiledTexture>> m_Textures;
};

ETH_NAMESPACE_END

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int cmdShow)
{
    CreateMutexA(0, false, "Local\\Ether.Toolmode");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        MessageBox(nullptr, L"An instance of Ether is already running.", L"Ether", MB_OK | MB_ICONEXCLAMATION);
        return EXIT_FAILURE;
    }

    return Start(Ether::EtherToolmode());
}

