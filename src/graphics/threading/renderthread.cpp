/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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

#include "graphics/threading/renderthread.h"
#include "graphics/graphiccore.h"

Ether::Graphics::RenderThread::RenderThread()
{
    m_GraphicsThreadEnabled = GraphicCore::GetGraphicConfig().GetUseGraphicsThread();

    if (m_GraphicsThreadEnabled)
    {
        m_GraphicsThreadRunning = true;
        m_ShutdownRequested = false;

        m_GraphicsThread = std::make_unique<std::thread>(&RenderThread::MainRenderLoop, this);
        m_GraphicsThreadId = m_GraphicsThread->get_id();
    }
}

Ether::Graphics::RenderThread::~RenderThread()
{
    if (!m_GraphicsThreadRunning)
        return;

    m_ShutdownRequested = true;

    // Wake up the thread
    {
        std::lock_guard<std::mutex> lock(m_FrameMutex);
        m_RenderDataReady = true;
    }
    m_FrameCV.notify_one();

    if (m_GraphicsThread && m_GraphicsThread->joinable())
        m_GraphicsThread->join();

    m_RenderCommandQueue.Clear();
    m_GraphicsThreadRunning = false;
}

bool Ether::Graphics::RenderThread::IsGraphicsThread()
{
    if (!m_GraphicsThreadEnabled)
        return false;

    return std::this_thread::get_id() == m_GraphicsThreadId;
}

bool Ether::Graphics::RenderThread::IsGraphicsThreadEnabled()
{
    return m_GraphicsThreadEnabled;
}

void Ether::Graphics::RenderThread::EnqueueRenderCommand(std::function<void()> command)
{
    if (!IsGraphicsThreadEnabled() || !m_GraphicsThreadRunning)
    {
        command(); // Fallback to single-thread
        return;
    }

    m_RenderCommandQueue.Enqueue(command);
}

void Ether::Graphics::RenderThread::MainRenderLoop()
{
    ETH_MARKER_THREAD("Render Thread");

    while (!m_ShutdownRequested)
    {
        ETH_MARKER_FRAME("Graphics Frame");

        {
            std::unique_lock<std::mutex> lock(m_FrameMutex);
            m_FrameCV.wait(lock, [this] { return m_RenderDataReady.load() || m_ShutdownRequested.load(); });
            m_RenderDataReady = false;
        }

        if (m_ShutdownRequested)
            break;

        m_FrameComplete = false;

        ProcessRenderCommands();

        {
            std::lock_guard<std::mutex> lock(m_CompleteMutex);
            m_FrameComplete = true;
        }

        m_CompleteCV.notify_all();
    }
}

void Ether::Graphics::RenderThread::ProcessRenderCommands()
{
    RenderCommand command;
    while (m_RenderCommandQueue.TryDequeue(command))
    {
        if (command.m_Execute)
            command.m_Execute();
    }
}

void Ether::Graphics::RenderThread::SignalFrame()
{
    {
        std::lock_guard<std::mutex> lock(m_FrameMutex);
        m_RenderDataReady = true;
    }

    m_FrameCV.notify_one();
}

void Ether::Graphics::RenderThread::WaitForFrame()
{
    ETH_MARKER_EVENT("Wait for Render Thread Completion");

    std::unique_lock<std::mutex> lock(m_CompleteMutex);
    m_CompleteCV.wait(lock, [this] { return m_FrameComplete.load(); });
}

