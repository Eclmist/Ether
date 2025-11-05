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
#include "engine/enginecore.h"
#include <format>
#include <algorithm>
#include <filesystem>

using namespace Ether;

static constexpr KeyCode KeyCode_LoadScene = (KeyCode)Win32::KeyCode::F1;
static constexpr KeyCode KeyCode_ToggleDebugGui = (KeyCode)Win32::KeyCode::F3;
static constexpr KeyCode KeyCode_ToggleFullscreen = (KeyCode)Win32::KeyCode::F11;
static constexpr KeyCode KeyCode_ToggleRaytracingDebug = (KeyCode)Win32::KeyCode::Space;

void SampleApp::Initialize()
{
    LogInfo("Initializing Application: Sample App");
    Client::SetClientTitle("Ether Sample App");
    Client::SetClientSize({ 1920, 1080 });
}

void SampleApp::LoadContent()
{
    World& world = GetActiveWorld();
    const std::string worldToLoad = GetCommandLineOptions().GetWorldName();

    if (worldToLoad != "")
        world.Load(worldToLoad);

    Entity& cameraObj = world.CreateCamera();
    m_CameraTransform = &cameraObj.GetComponent<Ecs::EcsTransformComponent>();
    m_CameraTransform->m_Translation = { 0, 2, 0 };
    m_CameraTransform->m_Rotation = { 0, SMath::DegToRad(-90.0f), 0 };


    Ether::Graphics::GraphicConfig& graphicConfig = Ether::Graphics::GetGraphicConfig();

    graphicConfig.m_Fov = 7.0f;

    graphicConfig.m_BloomIntensity = 0.440f;
    graphicConfig.m_BloomScatter = 0.646;
    graphicConfig.m_BloomAnamorphic = 0.57f;

    graphicConfig.m_FocusDistance = 20.352;
    graphicConfig.m_Aperture = 8.241;
    graphicConfig.m_FocusRange = 12.814;

    graphicConfig.m_SunDirection = ethVector4(0.353, 0.590, 0.247, 1.0f).Normalized();
    graphicConfig.m_SunColor = ethVector4(254 / 255.0f, 200 / 255.0f, 142 / 255.0f, 1.0f);
    graphicConfig.m_Exposure = 0.00015;

    graphicConfig.m_ColorGrading_Temperature = -0.15;
    graphicConfig.m_BloomAnamorphic = 0.57f;

    graphicConfig.m_DebugJitterScale = 1.0f;

    m_CameraTransform->m_Translation = { 7.432314, 1.387823, -12.617146 };
    m_CameraTransform->m_Rotation = { 0.062000, -0.552797, 0.000000 };


}

void SampleApp::UnloadContent()
{
}

void SampleApp::Shutdown()
{
}

void SampleApp::OnUpdate(const UpdateEventArgs& e)
{
    UpdateGraphicConfig();
    UpdateCamera();
}

void SampleApp::OnRender(const RenderEventArgs& e)
{
}

void SampleApp::OnShutdown()
{
}

void SampleApp::UpdateGraphicConfig() const
{
    Ether::Graphics::GraphicConfig& graphicConfig = Ether::Graphics::GetGraphicConfig();

    if (Input::GetKeyDown(KeyCode_ToggleFullscreen))
        Ether::Client::SetFullscreen(!Ether::Client::IsFullscreen());

    if (Input::GetKeyDown(KeyCode_ToggleRaytracingDebug))
    {
        /*
        graphicConfig.m_RaytracingMode = (graphicConfig.m_RaytracingMode == Ether::Graphics::RaytracingMode::ReSTIR) 
            ? Ether::Graphics::RaytracingMode::Pathtrace 
            : Ether::Graphics::RaytracingMode::ReSTIR;
        */

        graphicConfig.m_IsRaytracingDebugEnabled ^= true;
    }

    if (Input::GetKeyDown(KeyCode_ToggleDebugGui))
        graphicConfig.SetDebugGuiEnabled(!graphicConfig.IsDebugGuiEnabled());


    if (Input::GetKey((KeyCode)Win32::KeyCode::J))
        graphicConfig.m_SunDirection = (graphicConfig.m_SunDirection + Ether::ethVector4(-1, 0, 0, 0) *

                                       Time::GetDeltaTime() * 0.0002).Normalized();
    if (Input::GetKey((KeyCode)Win32::KeyCode::L))
        graphicConfig.m_SunDirection = (graphicConfig.m_SunDirection + Ether::ethVector4(1, 0, 0, 0) *
                                       Time::GetDeltaTime() * 0.0002).Normalized();
    if (Input::GetKey((KeyCode)Win32::KeyCode::I))
        graphicConfig.m_SunDirection = (graphicConfig.m_SunDirection + Ether::ethVector4(0, 0, 1, 0) *
                                       Time::GetDeltaTime() * 0.0002).Normalized();
    if (Input::GetKey((KeyCode)Win32::KeyCode::K))
        graphicConfig.m_SunDirection = (graphicConfig.m_SunDirection + Ether::ethVector4(0, 0, -1, 0) *
                                       Time::GetDeltaTime() * 0.0002).Normalized();
    if (Input::GetKey((KeyCode)Win32::KeyCode::U))
        graphicConfig.m_SunDirection =
            (graphicConfig.m_SunDirection + Ether::ethVector4(0, 1, 0, 0) * Time::GetDeltaTime() * 0.0002);
    if (Input::GetKey((KeyCode)Win32::KeyCode::O))
        graphicConfig.m_SunDirection =
            (graphicConfig.m_SunDirection + Ether::ethVector4(0, -1, 0, 0) * Time::GetDeltaTime() * 0.0002);
}

void SampleApp::UpdateCamera() const
{
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
