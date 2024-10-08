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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace Ether;

static constexpr KeyCode KeyCode_LoadScene = (KeyCode)Win32::KeyCode::F1;
static constexpr KeyCode KeyCode_ToggleDebugGui = (KeyCode)Win32::KeyCode::F3;
static constexpr KeyCode KeyCode_ToggleFullscreen = (KeyCode)Win32::KeyCode::F11;
static constexpr KeyCode KeyCode_ToggleRaytracingDebug = (KeyCode)Win32::KeyCode::Space;

static constexpr uint32_t ResolutionWidth = 1280;
static constexpr uint32_t ResolutionHeight = 720;
static constexpr uint32_t RtCampMaxFrames = 300;
static constexpr uint32_t RtCampNumAccumulationFrames = 180;

// RTCamp Camera Waypoints
static const int32_t numPoints = 15;

static Ether::ethVector3 positions[numPoints] = {
    { 0.774253, 0.136848, -1.317562 },  { 0.561229, 0.151479, -1.376745 },  { 0.245344, 0.204299, -1.260417 },
    { -0.066522, 0.324728, -0.727548 }, { -0.042222, 0.478083, -0.107213 }, { 0.242863, 0.903185, 1.566718 },
    { 0.731789, 1.301521, 2.858444 },   { 1.973294, 1.697622, 5.045119 },   { 1.695005, 1.611027, 6.421093 },
    { 0.367149, 1.383249, 6.929018 },   { 0.106207, 1.239281, 6.519328 },   { 0.106207, 1.239281, 6.519328 },
    { 0.051574, 1.205059, 5.988041 },   { -0.095355, 1.247614, 5.974273 },  { -0.139254, 1.266647, 6.038291 }
};

static Ether::ethVector3 rotations[numPoints] = {
    { -0.160000, -0.689999, 0.000000 }, { -0.184000, -0.628000, 0.000000 }, { -0.196000, -0.450001, 0.000000 },
    { -0.242000, 0.037999, 0.000000 },  { -0.244000, 0.055999, 0.000000 },  { -0.236000, 0.288000, 0.000000 },
    { -0.276000, 0.386000, 0.000000 },  { -0.096000, -0.488001, 0.000000 }, { 0.158000, -1.413996, 0.000000 },
    { 0.282000, -2.782002, 0.000000 },  { 0.282000, -3.012006, 0.000000 },  { 0.282000, -3.012006, 0.000000 },
    { -0.256000, -1.484007, 0.000000 }, { -0.294000, -1.210009, 0.000000 }, { -0.322000, -1.428006, 0.000000 }
};




    void SampleApp::Initialize()
{
    LogInfo("Initializing Application: Sample App");
    Client::SetClientTitle("Ether Sample App");
    Client::SetClientSize({ ResolutionWidth, ResolutionHeight });

    m_OutImage.resize(4 * ResolutionWidth * ResolutionHeight);

    m_CommandLineOptions = Ether::GetCommandLineOptions();
}

void SampleApp::LoadContent()
{
    World& world = GetActiveWorld();
    const std::string worldToLoad = m_CommandLineOptions.GetWorldName();

    if (worldToLoad != "")
        world.Load(worldToLoad);

    Entity& cameraObj = world.CreateCamera();
    m_CameraTransform = &cameraObj.GetComponent<Ecs::EcsTransformComponent>();
    m_CameraTransform->m_Translation = { 0, 1, -1 };
    m_CameraTransform->m_Rotation = { 0, SMath::DegToRad(0.0f), 0 };


    Ether::Graphics::GraphicConfig& graphicConfig = Ether::Graphics::GetGraphicConfig();
    graphicConfig.m_SunDirection = { 0.713, 0.517, -0.473, 0.0f };
    graphicConfig.m_SunColor = { 255 / 255.0f,  130 / 255.0f, 56 / 255.0f, 0.0f };
}

void SampleApp::UnloadContent()
{
}

void SampleApp::Shutdown()
{
}

void SampleApp::OnUpdate(const UpdateEventArgs& e)
{
    World& world = GetActiveWorld();

    for (int i = 0; i < 10; ++i)
    {
        auto entity0 = world.GetEntity(i);
        auto e0transform = entity0.GetComponent<Ecs::EcsTransformComponent>();
        e0transform.m_Translation = sin(Time::GetCurrentTime() * Time::GetDeltaTime());
    }


    UpdateGraphicConfig();
    UpdateCamera();

    if (m_CommandLineOptions.m_RTCampMode && m_OutputFrameNumber >= RtCampMaxFrames)
    {
        LogInfo("%u frames exported", m_OutputFrameNumber);
        Ether::Shutdown();
    }

    static uint32_t numKeyframes = 0;

    if (Input::GetKeyDown((KeyCode)Win32::KeyCode::G))
    {
        numKeyframes++;
        LogInfo("TotalPoints: %u, \t translation: \t{%f, %f, %f}, \t Rotation: \t{%f, %f, %f},",
            numKeyframes,
                m_CameraTransform->m_Translation.x,
            m_CameraTransform->m_Translation.y < -5 ? m_CameraTransform->m_Translation.y + 20.0f
                                                    : m_CameraTransform->m_Translation.y,
                m_CameraTransform->m_Translation.z,
            m_CameraTransform->m_Rotation.x,
            m_CameraTransform->m_Rotation.y,
            m_CameraTransform->m_Rotation.z);
    }
}

void SampleApp::OnRender(const RenderEventArgs& e)
{
}

void SampleApp::OnPostRender()
{
    if (!m_CommandLineOptions.m_RTCampMode)
        return;

    m_LocalFrameNumber++;

    if (m_LocalFrameNumber % RtCampNumAccumulationFrames == 0)
    {
        if (m_OutputFrameNumber >= RtCampMaxFrames)
            return;

        void* pReadbackBufferData{};

        std::ostringstream filename;
        filename << std::setw(3) << std::setfill('0') << m_OutputFrameNumber << ".png";

        LogInfo("Exporting frame number %u", m_OutputFrameNumber);
        Ether::Graphics::Export(&pReadbackBufferData);

        stbi_write_png(
            filename.str().c_str(),
            ResolutionWidth,
            ResolutionHeight,
            4,
            pReadbackBufferData,
            4 * ResolutionWidth);

        m_OutputFrameNumber++;
    }
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
        graphicConfig.m_IsRaytracingDebugEnabled = !graphicConfig.m_IsRaytracingDebugEnabled;

    if (Input::GetKeyDown(KeyCode_ToggleDebugGui))
        graphicConfig.SetDebugGuiEnabled(!graphicConfig.IsDebugGuiEnabled());

    if (Input::GetKey((KeyCode)Win32::KeyCode::J))
        graphicConfig.m_SunDirection = (graphicConfig.m_SunDirection + Ether::ethVector4(-1, 0, 0, 0) *

                                                                           Time::GetDeltaTime() * 0.0002)
                                           .Normalized();
    if (Input::GetKey((KeyCode)Win32::KeyCode::L))
        graphicConfig.m_SunDirection = (graphicConfig.m_SunDirection +
                                        Ether::ethVector4(1, 0, 0, 0) * Time::GetDeltaTime() * 0.0002)
                                           .Normalized();
    if (Input::GetKey((KeyCode)Win32::KeyCode::I))
        graphicConfig.m_SunDirection = (graphicConfig.m_SunDirection +
                                        Ether::ethVector4(0, 0, 1, 0) * Time::GetDeltaTime() * 0.0002)
                                           .Normalized();
    if (Input::GetKey((KeyCode)Win32::KeyCode::K))
        graphicConfig.m_SunDirection = (graphicConfig.m_SunDirection +
                                        Ether::ethVector4(0, 0, -1, 0) * Time::GetDeltaTime() * 0.0002)
                                           .Normalized();
    if (Input::GetKey((KeyCode)Win32::KeyCode::U))
        graphicConfig.m_SunDirection =
            (graphicConfig.m_SunDirection + Ether::ethVector4(0, 1, 0, 0) * Time::GetDeltaTime() * 0.0002);
    if (Input::GetKey((KeyCode)Win32::KeyCode::O))
        graphicConfig.m_SunDirection =
            (graphicConfig.m_SunDirection + Ether::ethVector4(0, -1, 0, 0) * Time::GetDeltaTime() * 0.0002);
}

ethVector3 CatmullRomChordal(
    const ethVector3& p0,
    const ethVector3& p1,
    const ethVector3& p2,
    const ethVector3& p3,
    float t)
{
    float t2 = t * t;
    float t3 = t2 * t;

    return ((p1 * 2) + (-p0 + p2) * t + (p0 * 2 - p1 * 5 + p2 * 4 - p3) * t2 + (-p0 + p1 * 3 - p2 * 3 + p3) * t3) * 0.5;
}

ethVector3 LerpRotation(const ethVector3& r1, const ethVector3& r2, float t)
{
    return r1 * (1 - t) + r2 * t;
}

// Parametric function to get position and rotation at time t
void GetPositionAndRotation(
    float t,
    ethVector3& outPosition,
    ethVector3& outRotation)
{
    // Calculate segment index based on t
    float segmentFloat = t * (numPoints - 1);          // Normalized time
    int segmentIndex = static_cast<int>(segmentFloat); // Integer index for points
    float segmentT = segmentFloat - segmentIndex;      // Fractional part of t for interpolation

    // Wrap around boundary conditions for Catmull-Rom
    int p0 = std::max(segmentIndex - 1, 0);
    int p1 = segmentIndex;
    int p2 = std::min(segmentIndex + 1, numPoints - 1);
    int p3 = std::min(segmentIndex + 2, numPoints - 1);

    // Interpolate position using Catmull-Rom spline
    outPosition = CatmullRomChordal(positions[p0], positions[p1], positions[p2], positions[p3], segmentT);

    // Interpolate rotation using linear interpolation (lerp)
    outRotation = CatmullRomChordal(rotations[p0], rotations[p1], rotations[p2], rotations[p3], segmentT);
}


void SampleApp::UpdateCamera() const
{
    static float time = 0;

    if ((m_CommandLineOptions.m_RTCampMode && m_LocalFrameNumber % RtCampNumAccumulationFrames == 0)
        || m_CommandLineOptions.m_RTCampDebugMode)
    {

        //float time = (Time::GetCurrentTime() - Time::GetStartupTime()) / 10000.0f;

        //if (m_CommandLineOptions.m_RTCampMode)
        //{
        //    time /= RtCampNumAccumulationFrames;
        //    time *= 1.3;
        //}

        //while (time >= 1.0)
        //    time -= 1.0f;


        if (m_CommandLineOptions.m_RTCampDebugMode)
        {
            time += Time::GetDeltaTime();

            if (time >= 10000.0f)
                time -= 10000.0f;
        }
        else
            time = m_OutputFrameNumber / (float)RtCampMaxFrames;

        ethVector3 position, rotation;

        // Get interpolated position and rotation at time t
        GetPositionAndRotation(std::clamp(m_CommandLineOptions.m_RTCampDebugMode ? time / 10000.0f : time, 0.0f, 1.0f), position, rotation);

        m_CameraTransform->m_Translation = position;
        m_CameraTransform->m_Rotation = rotation;

        if (m_CameraTransform->m_Translation.z >= 0.188 && m_CameraTransform->m_Translation.y > -10)
        {
            m_CameraTransform->m_Translation.y -= 20;
        }

    }
    else
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
        float yOffset = 0;

        if (m_CameraTransform->m_Translation.z >= 0.188 && m_CameraTransform->m_Translation.y > -10)
        {
            yOffset = -20;
        }
        else if (m_CameraTransform->m_Translation.z <= 0.189 && m_CameraTransform->m_Translation.y <= -10)
        {
            yOffset = 20;
        }
        else
        {
            yOffset = 0;
        }

        m_CameraTransform->m_Translation.y += yOffset;
    }
}
