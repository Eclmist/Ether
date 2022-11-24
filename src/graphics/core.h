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
#include "graphics/rhi/rhiimguiwrapper.h"

#include "graphics/memory/descriptorallocator.h"
#include "graphics/memory/bindlessresourcemanager.h"

#include "graphics/config/graphicconfig.h"

#include "graphics/commandmanager.h"
#include "graphics/graphicdisplay.h"

namespace Ether::Graphics
{
    class ETH_GRAPHIC_DLL Core : public Singleton<Core>
    {
    public:
        Core() = default;
        ~Core();

    public:
        void Initialize(const GraphicConfig& config);

    public:
        static uint64_t inline GetFrameNumber() { return Instance().m_FrameNumber; }

    public:
        static inline BindlessResourceManager& GetBindlessResourceManager() { return *Instance().m_BindlessResourceManager; }
        static inline CommandManager& GetCommandManager() { return *Instance().m_CommandManager; }
        static inline DescriptorAllocator& GetGpuDescriptorAllocator() { return *Instance().m_GpuDescriptorAllocator; }
        static inline GraphicDisplay& GetGraphicsDisplay() { return *Instance().m_GraphicsDisplay; }
        static inline RhiDevice& GetDevice() { return *Instance().m_RhiDevice; }
        static inline RhiModule& GetModule() { return *Instance().m_RhiModule; }
        static inline RhiImguiWrapper& GetImguiWrapper() { return *Instance().m_Imgui; }

        static inline const GraphicConfig& GetConfig() { return Instance().m_Config; }

    public:
        static void Update();
        static void FlushGpu();

    private:
        std::unique_ptr<RhiModule> m_RhiModule;
        std::unique_ptr<RhiDevice> m_RhiDevice;
        std::unique_ptr<RhiImguiWrapper> m_Imgui;

        std::unique_ptr<BindlessResourceManager> m_BindlessResourceManager;
        std::unique_ptr<CommandManager> m_CommandManager;
        std::unique_ptr<DescriptorAllocator> m_GpuDescriptorAllocator;
        std::unique_ptr<GraphicDisplay> m_GraphicsDisplay;

    private:
        GraphicConfig m_Config;

        uint64_t m_FrameNumber;
    };
}

