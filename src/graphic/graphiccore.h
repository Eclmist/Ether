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

// Rendering Hardware Interfaces
#include "graphic/rhi/rhicommon.h"
#include "graphic/rhi/rhimodule.h"

// Allocators
#include "graphic/memory/bindlessresourcemanager.h"
#include "graphic/memory/descriptorallocator.h"

// Contexts
#include "graphic/context/commandcontext.h"
#include "graphic/context/graphiccontext.h"
#include "graphic/context/resourcecontext.h"

// Common
#include "graphic/commandmanager.h"
#include "graphic/graphiccommon.h"
#include "graphic/graphicdisplay.h"
#include "graphic/graphicrenderer.h"

// Shader Daemon
#include "graphic/common/shaderdaemon.h"

ETH_NAMESPACE_BEGIN

class ETH_ENGINE_DLL GraphicCore : public Singleton<GraphicCore>
{
public:
    GraphicCore() = default;
    ~GraphicCore() = default;

    static void Initialize();
    static void Render();
    static void Shutdown();
    static bool IsInitialized();

    static inline BindlessResourceManager& GetBindlessResourceManager() { return *Instance().m_BindlessResourceManager; }
    static inline CommandManager& GetCommandManager() { return *Instance().m_CommandManager; }
    static inline DescriptorAllocator& GetGpuDescriptorAllocator() { return *Instance().m_GpuDescriptorAllocator; }
    static inline GraphicCommon& GetGraphicCommon() { return *Instance().m_GraphicCommon; }
    static inline GraphicDisplay& GetGraphicDisplay() { return *Instance().m_GraphicDisplay; }
    static inline GraphicRenderer& GetGraphicRenderer() { return *Instance().m_GraphicRenderer; }
    static inline ShaderDaemon& GetShaderDaemon() { return *Instance().m_ShaderDaemon; }

    static inline RhiDeviceHandle GetDevice() { return Instance().m_RhiDevice; }
    static inline RhiModuleHandle GetModule() { return Instance().m_RhiModule; }

    static inline uint64_t GetFrameNumber() { return Instance().m_FrameNumber; }

public:
    static void FlushGpu();

private:
    std::unique_ptr<BindlessResourceManager> m_BindlessResourceManager;
    std::unique_ptr<CommandManager> m_CommandManager;
    std::unique_ptr<DescriptorAllocator> m_GpuDescriptorAllocator;
    std::unique_ptr<GraphicCommon> m_GraphicCommon;
    std::unique_ptr<GraphicDisplay> m_GraphicDisplay;
    std::unique_ptr<GraphicRenderer> m_GraphicRenderer;
    std::unique_ptr<GuiRenderer> m_GuiRenderer;
    std::unique_ptr<ShaderDaemon> m_ShaderDaemon;

    RhiDeviceHandle m_RhiDevice;
    RhiModuleHandle m_RhiModule;

private:
    bool m_IsInitialized;

    uint64_t m_FrameNumber;
};

ETH_NAMESPACE_END

