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
static constexpr uint32_t RtCampNumAccumulationFrames = 120;

// RTCamp Camera Waypoints
static const int32_t numPoints = 15;

static Ether::ethVector3 positions[numPoints] = {
    { -3.655200, 1.060171, -0.987890 },  { -2.192420, 0.582169, -2.934098 },  { -2.381783, 0.616636, -1.212492 },
    { -1.795741, 0.686278, 0.310832 },   { 0.418850, 1.009112, 0.319021 },    { 2.282794, 1.851532, 0.049275 },
    { 4.612771, 1.710831, -1.083092 }, { 6.249527, 1.710831, -1.623956 }, { 6.060591, 1.617119, 0.470194 },
    { 3.737294, 1.450020, 0.435412 },  { 0.701626, 1.577809, -0.181008 },   { -1.051766, 1.582359, -2.086463 },
    { -2.953543, 1.556680, -2.530478 },  { -3.214054, 1.501628, -0.468931 },  { -2.393553, 1.494153, 0.936790 }
};
static Ether::ethVector3 rotations[numPoints] = {
    { 0.015999, 1.498001, 0.000000 },   { -0.026001, 1.124005, 0.000000 },  { -0.124001, 1.358002, 0.000000 },
    { -0.154001, 1.837996, 0.000000 },  { -0.088001, 1.879996, 0.000000 },  { -0.074001, 1.568000, 0.000000 },
    { -0.052001, 1.280004, 0.000000 },  { -0.080001, -0.551996, 0.000000 }, { -0.072001, -1.569990, 0.000000 },
    { -0.066001, -1.635990, 0.000000 }, { 0.005999, -1.885986, 0.000000 },  { 0.011999, -1.281991, 0.000000 },
    { 0.035999, -0.045992, 0.000000 },  { 0.009999, 1.436008, 0.000000 },   { 0.023999, 1.990005, 0.000000 },
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
                m_CameraTransform->m_Translation.y,
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
    static float startTime = Time::GetCurrentTime();

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

        float time = m_OutputFrameNumber / (float)RtCampMaxFrames;

        ethVector3 position, rotation;

        // Get interpolated position and rotation at time t
        GetPositionAndRotation(std::clamp(time, 0.0f, 1.0f), position, rotation);

        m_CameraTransform->m_Translation = position;
        m_CameraTransform->m_Rotation = rotation;

        if (m_CameraTransform->m_Translation.x >= 1.369 && m_CameraTransform->m_Translation.y > -10)
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


        if (m_CameraTransform->m_Translation.x >= 1.367 && m_CameraTransform->m_Translation.y > -10)
        {
            yOffset = -20;
        }
        else if (m_CameraTransform->m_Translation.x <= 1.369 && m_CameraTransform->m_Translation.y <= -10)
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
