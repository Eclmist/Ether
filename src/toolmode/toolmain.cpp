/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <string>
#include "api/api.h"

ETH_NAMESPACE_BEGIN

using namespace DirectX;

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
        Entity* debugCube = EngineCore::GetECSManager().CreateEntity("AssetImportSlot (Temp)");

		// TODO: Create default material somewhere, maybe in graphiccommon, or maybe serialized somewhere?
        m_Material = std::make_shared<Material>();

        debugCube->AddComponent<MeshComponent>();
        debugCube->AddComponent<VisualComponent>();
        debugCube->GetComponent<VisualComponent>()->SetMaterial(m_Material);
        debugCube->GetComponent<TransformComponent>()->SetPosition({ 0, 0, 0 });
        debugCube->GetComponent<TransformComponent>()->SetRotation({ 0, 0, 0 });

        for (int i = 0; i < 10; ++i)
        {
            Entity* newEntity = EngineCore::GetECSManager().CreateEntity("Cube");
            newEntity->GetComponent<TransformComponent>()->SetPosition({ (float)(rand() % 100 - 50), (float)(rand() % 100 - 50), (float)(rand() % 100 - 50) });
            newEntity->GetComponent<TransformComponent>()->SetRotation({ (float)rand(), (float)rand(), (float)rand() });
            newEntity->GetComponent<TransformComponent>()->SetScale({ 1.5, 1.5, 1.5 });
            newEntity->AddComponent<VisualComponent>();
            newEntity->GetComponent<VisualComponent>()->SetMaterial(m_Material);
            newEntity->AddComponent<MeshComponent>();
        }
    };

    void UnloadContent() override {};
    void Shutdown() override {};

public:
    void OnUpdate(const UpdateEventArgs& e) override
    {
        if (Input::GetKeyDown(Win32::KeyCode::F3))
            EngineCore::GetEngineConfig().ToggleDebugGui();

        if (Input::GetKeyDown(Win32::KeyCode::F11))
            EngineCore::GetMainWindow().SetFullscreen(!EngineCore::GetMainWindow().GetFullscreen());

        if (Input::GetKeyDown(Win32::KeyCode::F1))
            m_CameraMode = (CameraMode)(((uint32_t)m_CameraMode + 1) % (uint32_t)CameraMode::NumModes);

        UpdateCamera(e.m_DeltaTime);

        float x = sin((float)GetTimeSinceStart());
        EngineCore::GetECSManager().GetEntity(1)->GetComponent<TransformComponent>()->SetPosition({ x, 5, 5 });
        EngineCore::GetECSManager().GetEntity(1)->SetName("Debug Cube (" + std::to_string(x) + ")");
    };

    void OnRender(const RenderEventArgs& e) override
    {
        const ethXMVector upDir = XMVectorSet(0, 1, 0, 0);
        ethMatrix4x4 viewMatrixRaw;
        ethMatrix4x4 viewMatrixInvRaw;
        ethMatrix4x4 projMatrixRaw;
        XMStoreFloat4x4(&viewMatrixRaw, m_ViewMatrix);
        XMStoreFloat4x4(&viewMatrixInvRaw, m_ViewMatrixInv);
        XMStoreFloat4x4(&projMatrixRaw, m_ProjectionMatrix);

        e.m_GraphicContext->SetViewMatrix(viewMatrixRaw);
        e.m_GraphicContext->SetProjectionMatrix(projMatrixRaw);
        e.m_GraphicContext->SetEyeDirection({ viewMatrixRaw._13, viewMatrixRaw._23, viewMatrixRaw._33, 1.0 });
        e.m_GraphicContext->SetEyePosition({ viewMatrixInvRaw._41, viewMatrixInvRaw._42, viewMatrixInvRaw._43, 0.0 });
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
            m_CameraRotation.x = std::clamp(m_CameraRotation.x, -XMConvertToRadians(90), XMConvertToRadians(90));
        }

        ethXMMatrix rotationMatrix = XMMatrixRotationY(m_CameraRotation.y) * XMMatrixRotationX(m_CameraRotation.x);
        ethXMMatrix translationMatrix = XMMatrixTranslation(0, 0, m_CameraDistance);

        m_ViewMatrix = rotationMatrix * translationMatrix;
        m_ViewMatrixInv = XMMatrixInverse(nullptr, m_ViewMatrix);

        float aspectRatio = EngineCore::GetEngineConfig().GetClientWidth() / static_cast<float>(EngineCore::GetEngineConfig().GetClientHeight());
        m_ProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(80), aspectRatio, 0.01f, 1000.0f);
    }

    void UpdateFlyCam(float deltaTime)
    {
        if (Input::GetMouseButton(2))
        {
			m_CameraRotation.x -= Input::GetMouseDeltaY() / 500;
			m_CameraRotation.y -= Input::GetMouseDeltaX() / 500;
			m_CameraRotation.x = std::clamp(m_CameraRotation.x, -XMConvertToRadians(90), XMConvertToRadians(90));
        }

        ethXMMatrix rotationMatrix = XMMatrixRotationY(m_CameraRotation.y) * XMMatrixRotationX(m_CameraRotation.x);
        ethXMVector forwardVec = XMVector4Normalize(XMVector3Transform(g_XMIdentityR2, XMMatrixTranspose(rotationMatrix)));
        ethXMVector upVec = XMVector4Normalize(g_XMIdentityR1);
        ethXMVector rightVec = XMVector4Normalize(XMVector3Cross(upVec, forwardVec));

        if (Input::GetMouseButton(2))
        {
			if (Input::GetKey(Win32::KeyCode::W))
				m_CameraPosition = XMVectorSubtract(m_CameraPosition, forwardVec * deltaTime * 20);
			if (Input::GetKey(Win32::KeyCode::S))
				m_CameraPosition = XMVectorAdd(m_CameraPosition, forwardVec * deltaTime * 20);
			if (Input::GetKey(Win32::KeyCode::A))
				m_CameraPosition = XMVectorAdd(m_CameraPosition, rightVec * deltaTime * 20);
			if (Input::GetKey(Win32::KeyCode::D))
				m_CameraPosition = XMVectorSubtract(m_CameraPosition, rightVec * deltaTime * 20);
        }

        m_CameraPosition = XMVectorSubtract(m_CameraPosition, forwardVec * Input::GetMouseWheelDelta() / 121.0);

        m_ViewMatrix = XMMatrixTranslationFromVector(m_CameraPosition) * rotationMatrix;
        m_ViewMatrixInv = XMMatrixInverse(nullptr, m_ViewMatrix);

        float aspectRatio = EngineCore::GetEngineConfig().GetClientWidth() / static_cast<float>(EngineCore::GetEngineConfig().GetClientHeight());
        m_ProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(80), aspectRatio, 0.01f, 1000.0f);
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

        ethXMMatrix rotationMatrix = XMMatrixRotationX(-90 * 0.0174533);
        ethXMMatrix translationMatrix = XMMatrixTranslation(m_OrthoX, m_OrthoZ, m_CameraDistance);
        m_ViewMatrix = rotationMatrix * translationMatrix;
        m_ViewMatrixInv = XMMatrixInverse(nullptr, m_ViewMatrix);
        m_ProjectionMatrix = XMMatrixOrthographicLH(
            (float)EngineCore::GetEngineConfig().GetClientWidth() * m_CameraDistance * scaleModifier,
            (float)EngineCore::GetEngineConfig().GetClientHeight() * m_CameraDistance * scaleModifier,
            0.01f, 1000.0f);
    }

    void ResetMatrices()
    {
        m_ViewMatrix = XMMatrixIdentity();
        m_ViewMatrixInv = XMMatrixIdentity();
        m_ProjectionMatrix = XMMatrixIdentity();
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
    ethXMVector m_CameraPosition;

    // Ortho Cam
    ethVector2 m_DragStartPos;
    float m_OrthoX, m_OrthoZ;

    ethXMMatrix m_ViewMatrix;
    ethXMMatrix m_ViewMatrixInv;
    ethXMMatrix m_ProjectionMatrix;


    std::shared_ptr<Material> m_Material;
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

    return Start(Ether::EtherToolmode(), cmdShow);
}

