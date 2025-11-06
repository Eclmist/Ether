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

#pragma once

#include "ether.h"
#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "engine/world/ecs/components/ecstransformcomponent.h"

class RTCamp11 : public Ether::IApplicationBase
{
public:
    void Initialize() override;
    void LoadContent() override;
    void UnloadContent() override;
    void Shutdown() override;

public:
    void OnUpdate(const Ether::UpdateEventArgs& e) override;
    void OnPreRender(const Ether::RenderEventArgs& e) override;
    void OnPostRender() override;
    void OnShutdown() override;

private:
    void UpdateGraphicConfig() const;
    void UpdateCamera() const;

private:
    Ether::Ecs::EcsTransformComponent* m_CameraTransform;
};

struct ExportJob
{
    std::string filename;
    std::vector<uint8_t> pixels;
    uint32_t width;
    uint32_t height;
};

class FrameExportWorker
{
public:
    FrameExportWorker()
        : m_Running(true)
        , m_Thread(&FrameExportWorker::WorkerMain, this)
    {
    }

    ~FrameExportWorker()
    {
        {
            std::unique_lock lock(m_Mutex);
            m_Running = false;
        }
        m_Cond.notify_one();
        m_Thread.join();
    }

    void Enqueue(ExportJob job)
    {
        {
            std::unique_lock lock(m_Mutex);
            m_Queue.push(std::move(job));
        }
        m_Cond.notify_one();
    }

private:
    void WorkerMain();

private:
    std::thread m_Thread;
    std::mutex m_Mutex;
    std::condition_variable m_Cond;
    std::queue<ExportJob> m_Queue;
    bool m_Running;
};
