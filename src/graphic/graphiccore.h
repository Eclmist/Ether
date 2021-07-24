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

// ComPtr library
#include <wrl.h>
namespace wrl = Microsoft::WRL;

// D3D12 library
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3d12shader.h>
#include <dxcapi.h>
#include <DirectXMath.h>

// D3D12 extension library
#include <d3dx12/d3dx12.h>

// Ether Graphics Library
#include "graphic/pipeline/graphicpipelinestate.h"
#include "graphic/pipeline/rootsignature.h"

#include "graphic/commandmanager.h"
#include "graphic/graphiccommon.h"
#include "graphic/graphiccontext.h"
#include "graphic/graphicdisplay.h"
#include "graphic/graphicrenderer.h"

#include "graphic/common/shaderdaemon.h"

ETH_NAMESPACE_BEGIN

class GraphicCore : public Singleton<GraphicCore>
{
public:
    static void Initialize();
    static void Render();
    static void Shutdown();
    static bool IsInitialized();

    static CommandManager& GetCommandManager() { return *Instance().m_CommandManager; }
    static GraphicCommon& GetGraphicCommon() { return *Instance().m_GraphicCommon; }
    static GraphicDisplay& GetGraphicDisplay() { return *Instance().m_GraphicDisplay; }
    static GraphicRenderer& GetGraphicRenderer() { return *Instance().m_GraphicRenderer; }
    static ShaderDaemon& GetShaderDaemon() { return *Instance().m_ShaderDaemon; }

    static IDXGIAdapter4& GetAdapter() { return *Instance().m_Adapter.Get(); }
    static ID3D12Device3& GetDevice() { return *Instance().m_GraphicDevice.Get(); }

public:
    static void FlushGPU();

private:
    void InitializeDebugLayer();
    void InitializeAdapter();
    void InitializeDevice();

private:
    std::unique_ptr<CommandManager> m_CommandManager;
    std::unique_ptr<GraphicCommon> m_GraphicCommon;
    std::unique_ptr<GraphicDisplay> m_GraphicDisplay;
    std::unique_ptr<GraphicRenderer> m_GraphicRenderer;
    std::unique_ptr<GuiRenderer> m_GuiRenderer;
    std::unique_ptr<ShaderDaemon> m_ShaderDaemon;

    wrl::ComPtr<IDXGIAdapter4> m_Adapter;
    wrl::ComPtr<ID3D12Device3> m_GraphicDevice;

private:
    bool m_IsInitialized;
};

ETH_NAMESPACE_END

