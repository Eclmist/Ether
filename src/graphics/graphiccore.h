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

#include "graphics/pch.h"

#include "graphics/rhi/rhicommandlist.h"
#include "graphics/rhi/rhimodule.h"
#include "graphics/rhi/rhidevice.h"
#include "graphics/rhi/rhidescriptorheap.h"

#include "graphics/command/commandmanager.h"
#include "graphics/common/visualbatch.h"
#include "graphics/config/graphicconfig.h"
#include "graphics/memory/descriptorallocator.h"
#include "graphics/memory/bindlessdescriptormanager.h"
#include "graphics/shaderdaemon/shaderdaemon.h"
#include "graphics/schedule/rendergraph/rendergraphmanager.h"

#include "graphics/graphiccommon.h"
#include "graphics/graphicdisplay.h"
#include "graphics/graphicrenderer.h"

namespace Ether::Graphics
{
class ETH_GRAPHIC_DLL GraphicCore : public Singleton<GraphicCore>
{
public:
    GraphicCore() = default;
    ~GraphicCore() = default;

public:
    void Initialize();
    void Shutdown();

public:
    static inline RhiDevice& GetDevice() { return *Instance().m_RhiDevice; }
    static inline RhiModule& GetModule() { return *Instance().m_RhiModule; }

    static inline BindlessDescriptorManager& GetBindlessDescriptorManager() { return *Instance().m_BindlessDescriptorManager; }
    static inline CommandManager& GetCommandManager() { return *Instance().m_CommandManager; }
    static inline DescriptorAllocator& GetRtvAllocator() { return *Instance().m_RtvAllocator; }
    static inline DescriptorAllocator& GetDsvAllocator() { return *Instance().m_DsvAllocator; }
    static inline DescriptorAllocator& GetSrvCbvUavAllocator() { return *Instance().m_SrvCbvUavAllocator; }
    static inline GraphicConfig& GetGraphicConfig() { return Instance().m_Config; }
    static inline GraphicCommon& GetGraphicCommon() { return *Instance().m_GraphicCommon; }
    static inline GraphicDisplay& GetGraphicDisplay() { return *Instance().m_GraphicDisplay; }
    static inline GraphicRenderer& GetGraphicRenderer() { return *Instance().m_GraphicRenderer; }
    static inline RenderGraphManager& GetRenderGraphManager() { return *Instance().m_RenderGraphManager; }
    static inline ShaderDaemon& GetShaderDaemon() { return *Instance().m_ShaderDaemon; }

public:
    static void Main();
    static void FlushGpu();

private:
    std::unique_ptr<RhiModule> m_RhiModule;
    std::unique_ptr<RhiDevice> m_RhiDevice;

    std::unique_ptr<BindlessDescriptorManager> m_BindlessDescriptorManager;
    std::unique_ptr<CommandManager> m_CommandManager;
    std::unique_ptr<DescriptorAllocator> m_RtvAllocator;
    std::unique_ptr<DescriptorAllocator> m_DsvAllocator;
    std::unique_ptr<DescriptorAllocator> m_SrvCbvUavAllocator;
    std::unique_ptr<GraphicCommon> m_GraphicCommon;
    std::unique_ptr<GraphicDisplay> m_GraphicDisplay;
    std::unique_ptr<GraphicRenderer> m_GraphicRenderer;
    std::unique_ptr<RenderGraphManager> m_RenderGraphManager;
    std::unique_ptr<ShaderDaemon> m_ShaderDaemon;

private:
    GraphicConfig m_Config;
};
} // namespace Ether::Graphics
