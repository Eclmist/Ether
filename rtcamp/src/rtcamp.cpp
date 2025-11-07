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

#include "rtcamp.h"
#include "engine/world/ecs/components/ecsvisualcomponent.h"
#include "engine/world/ecs/components/ecscameracomponent.h"
#include "engine/enginecore.h"
#include <format>
#include <algorithm>
#include <filesystem>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace Ether;

static constexpr KeyCode KeyCode_ToggleDebugGui = (KeyCode)Win32::KeyCode::F3;
static constexpr KeyCode KeyCode_ToggleFullscreen = (KeyCode)Win32::KeyCode::F11;
static constexpr KeyCode KeyCode_ToggleRaytracingDebug = (KeyCode)Win32::KeyCode::Space;

static constexpr float TotalTimeBudgetMS = 175000;      // 175 seconds (RTCamp11 rule) + 5 second buffer
static constexpr float TotalMovieTimeMS = 10000;        // 10 seconds
static constexpr uint32_t MovieFramesPerSecond = 24;    // 24 fps looks more filmic
static constexpr uint32_t NumFramesToExport = MovieFramesPerSecond * (TotalMovieTimeMS / 1000.0f);

bool g_ShouldExportMovie = false;
void* g_ExportBufferData = nullptr;
uint32_t g_MovieFrameNumber = 0;
bool g_ExportQueued = false;
float g_LastExportTime = 0;
float g_RunningFrameBudget = TotalTimeBudgetMS / NumFramesToExport;

FrameExportWorker g_FrameExportWorker;

void RTCamp11::Initialize()
{
    LogInfo("Initializing Application: RTCamp");
    Client::SetClientTitle("Raytracing Camp 11!!");
    Client::SetClientSize({ 640, 1080 });

    g_ShouldExportMovie = GetCommandLineOptions().GetExportMovie();
}

void RTCamp11::LoadContent()
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

    graphicConfig.m_RaytracingMode = Ether::Graphics::RaytracingMode::ReSTIR;

    m_CameraTransform->m_Translation = { 7.671061, 0.412040, -12.706130 };
    m_CameraTransform->m_Rotation = { 0.014000, -0.574797, 0.000000 };

    // Prewarm the first frame
    g_LastExportTime = Ether::Time::GetRealTimeSinceStartup();
    g_RunningFrameBudget = (TotalTimeBudgetMS - g_LastExportTime) / NumFramesToExport;
}

void RTCamp11::UnloadContent()
{
}

void RTCamp11::Shutdown()
{
}

void RTCamp11::OnUpdate(const UpdateEventArgs& e)
{
    UpdateGraphicConfig();
    UpdateCamera();
}

void RTCamp11::OnPreRender(const RenderEventArgs& e)
{
    ETH_MARKER_EVENT("OnPreRender()");

    if (g_ShouldExportMovie)
    {
        const float currentTimeMS = Ether::Time::GetRealTimeSinceStartup();
        const int32_t framesLeft = NumFramesToExport - g_MovieFrameNumber;

        if (framesLeft <= 0)
        {
            LogInfo("Full movie exported! :))");

            g_FrameExportWorker.Join();
            Ether::Shutdown();
            return;
        }

        static bool bHasPrintedWarning = false;
        if (currentTimeMS > TotalTimeBudgetMS && !bHasPrintedWarning)
        {
            LogWarning("Time limit likely exceeded but there are still %d frames left :(", framesLeft);
            bHasPrintedWarning = true;
        }

        if (currentTimeMS - g_LastExportTime > g_RunningFrameBudget)
        {
            Ether::Graphics::RequestExport(&g_ExportBufferData);
            g_ExportQueued = true;
            g_LastExportTime = currentTimeMS;
            return;
        }
    }
}

void RTCamp11::OnPostRender()
{
    ETH_MARKER_EVENT("OnPostRender()");

    if (g_ShouldExportMovie)
    {
        if (!g_ExportQueued)
            return;

        g_ExportQueued = false;
        ethVector2u resolution = Client::GetClientSize();

        std::ostringstream filename;
        filename << std::setw(3) << std::setfill('0') << g_MovieFrameNumber << ".png";

        LogInfo("Queuing frame %u for export...", g_MovieFrameNumber);

        size_t dataSize = 4ull * resolution.x * resolution.y;
        std::vector<uint8_t> pixels(dataSize);
        memcpy(pixels.data(), g_ExportBufferData, dataSize);

        ExportJob job;
        job.filename = filename.str();
        job.width = resolution.x;
        job.height = resolution.y;
        job.pixels = std::move(pixels);

        g_FrameExportWorker.Enqueue(std::move(job));

        g_MovieFrameNumber++;

        // Update time budget dynamically in case we hitch or some frames took longer
        //g_RunningFrameBudget = (TotalTimeBudgetMS - Ether::Time::GetRealTimeSinceStartup()) /
        //                       (g_MovieFrameNumber - NumFramesToExport);
    }
}

void RTCamp11::OnShutdown()
{
}

void RTCamp11::UpdateGraphicConfig() const
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

void RTCamp11::UpdateCamera() const
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

    if (Input::GetKey((KeyCode)Win32::KeyCode::P))
    {
        LogInfo(
            "Camera Translation: %f, %f, %f; Camera Rotation: %f, %f, %f",
            m_CameraTransform->m_Translation.x,
            m_CameraTransform->m_Translation.y,
            m_CameraTransform->m_Translation.z,
            m_CameraTransform->m_Rotation.x,
            m_CameraTransform->m_Rotation.y,
            m_CameraTransform->m_Rotation.z);
    }
}

void FrameExportWorker::WorkerMain()
{
    ETH_MARKER_THREAD("Export Thread");

    while (true)
    {
        ExportJob job;
        {
            std::unique_lock lock(m_Mutex);
            m_Cond.wait(lock, [&] { return !m_Queue.empty() || !m_Running; });

            if (!m_Running && m_Queue.empty())
                break;

            job = std::move(m_Queue.front());
            m_Queue.pop();
        }

        ETH_MARKER_EVENT("STBI Image Write");
        stbi_write_png(job.filename.c_str(), job.width, job.height, 4, job.pixels.data(), 4 * job.width);
    }
}
