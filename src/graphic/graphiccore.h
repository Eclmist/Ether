/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel Van Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

// Rendering Hardware Interfaces
#include "graphic/rhi/rhicommon.h"
#include "graphic/rhi/rhimodule.h"

// Contexts
#include "graphic/context/commandcontext.h"
#include "graphic/context/graphiccontext.h"

// Common
#include "graphic/commandmanager.h"
#include "graphic/graphiccommon.h"
#include "graphic/graphicdisplay.h"
#include "graphic/graphicrenderer.h"

// Shader Daemon
#include "graphic/common/shaderdaemon.h"

// Scheduler
#include "graphic/schedule/graphicscheduler.h"

ETH_NAMESPACE_BEGIN

class GraphicCore : public Singleton<GraphicCore>
{
public:
    GraphicCore() = default;
    ~GraphicCore() = default;

    static void Initialize();
    static void Render();
    static void Shutdown();
    static bool IsInitialized();

    static CommandManager& GetCommandManager() { return *Instance().m_CommandManager; }
    static GraphicCommon& GetGraphicCommon() { return *Instance().m_GraphicCommon; }
    static GraphicDisplay& GetGraphicDisplay() { return *Instance().m_GraphicDisplay; }
    static GraphicRenderer& GetGraphicRenderer() { return *Instance().m_GraphicRenderer; }
    static GraphicScheduler& GetGraphicScheduler() { return *Instance().m_GraphicScheduler; }
    static ShaderDaemon& GetShaderDaemon() { return *Instance().m_ShaderDaemon; }

    static RHIDeviceHandle GetDevice() { return Instance().m_RHIDevice; }
    static RHIModuleHandle GetModule() { return Instance().m_RHIModule; }

    // TODO: Move away from here into some dynamic descriptor heap allocator or something
    static inline RHIDescriptorHeapHandle GetRTVDescriptorHeap() { return Instance().m_RTVDescriptorHeap; }
    static inline RHIDescriptorHeapHandle GetDSVDescriptorHeap() { return Instance().m_DSVDescriptorHeap; }
    static inline RHIDescriptorHeapHandle GetSRVDescriptorHeap() { return Instance().m_SRVDescriptorHeap; }
    static inline RHIDescriptorHeapHandle GetSamplerDescriptorHeap() { return Instance().m_SamplerDescriptorHeap; }

public:
    static void FlushGpu();

private:
    void InitializeDebugLayer();

private:
    std::unique_ptr<CommandManager> m_CommandManager;
    std::unique_ptr<GraphicCommon> m_GraphicCommon;
    std::unique_ptr<GraphicDisplay> m_GraphicDisplay;
    std::unique_ptr<GraphicRenderer> m_GraphicRenderer;
    std::unique_ptr<GraphicScheduler> m_GraphicScheduler;
    //std::unique_ptr<GuiRenderer> m_GuiRenderer;
    std::unique_ptr<ShaderDaemon> m_ShaderDaemon;

    RHIDeviceHandle m_RHIDevice;
    RHIModuleHandle m_RHIModule;

    // TODO: Move away from here into some dynamic descriptor heap allocator or something
	RHIDescriptorHeapHandle m_RTVDescriptorHeap;
    RHIDescriptorHeapHandle m_DSVDescriptorHeap;
    RHIDescriptorHeapHandle m_SRVDescriptorHeap;
    RHIDescriptorHeapHandle m_SamplerDescriptorHeap;

private:
    bool m_IsInitialized;
};

ETH_NAMESPACE_END

