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

#pragma once

#include "graphics/pch.h"
#include "graphics/threading/rendercommandqueue.h"
#include <functional>

namespace Ether::Graphics
{
class RenderThread : public NonCopyable, public NonMovable
{
public:
    RenderThread();
    ~RenderThread();

public:
    ETH_GRAPHIC_DLL bool IsGraphicsThread();
    ETH_GRAPHIC_DLL bool IsGraphicsThreadEnabled();
    ETH_GRAPHIC_DLL void EnqueueRenderCommand(std::function<void()> command);

public:
    inline bool IsFrameComplete() const { return m_FrameComplete.load(); }

public:
    void MainRenderLoop();

public:
    void ProcessRenderCommands();
    void SignalFrame();
    void WaitForFrame();

private:
    std::unique_ptr<std::thread> m_GraphicsThread;
    std::thread::id m_GraphicsThreadId;
    RenderCommandQueue m_RenderCommandQueue;

    std::atomic<bool> m_GraphicsThreadEnabled = false;
    std::atomic<bool> m_GraphicsThreadRunning = false;
    std::atomic<bool> m_ShutdownRequested = false;

    std::mutex m_FrameMutex;
    std::condition_variable m_FrameCV;
    std::atomic<bool> m_RenderDataReady = false;

    std::mutex m_CompleteMutex;
    std::condition_variable m_CompleteCV;
    std::atomic<bool> m_FrameComplete = true;
};
} // namespace Ether::Graphics
