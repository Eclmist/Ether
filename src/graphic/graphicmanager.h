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

#include "graphic/commandmanager.h"
#include "graphic/graphicbatchrenderer.h"
#include "graphic/resource/textureresource.h"
#include "graphic/gui/guimanager.h"

ETH_NAMESPACE_BEGIN

class GraphicManager : public NonCopyable
{
public:
    void Initialize();
    void Shutdown();
    void WaitForPresent();
    void Render();
    void RenderGui();
    void Present();

    inline GraphicContext& GetGraphicContext() { return m_Context; }
    inline GraphicDisplay& GetGraphicDisplay() { return m_Display; }
    inline GuiManager& GetGuiManager() { return m_GuiManager; }

private:
    void CleanUp();
    void InitializeDebugLayer();
    void InitializeAdapter();
    void InitializeDevice();

private:
    wrl::ComPtr<IDXGIAdapter4> m_Adapter;

    GraphicBatchRenderer m_BatchRenderer;
    GraphicContext m_Context;
    GraphicDisplay m_Display;
    GuiManager m_GuiManager;
};

ETH_NAMESPACE_END