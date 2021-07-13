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
#include "graphic/gui/guimanager.h"

ETH_NAMESPACE_BEGIN

class GraphicRenderer : public NonCopyable
{
public:
    GraphicRenderer();
    ~GraphicRenderer();

    void WaitForPresent();
    void Render();
    void RenderGui();
    void Present();

    //TODO: maybe move into batch renderer?
    void RegisterVisual(Visual* visual);
    void DeregisterVisual(Visual* visual);

    inline GraphicContext& GetGraphicContext() { return m_Context; }
    inline GuiManager& GetGuiManager() { return m_GuiManager; }

private:
    void CleanUp();

private:
    GraphicBatchRenderer m_BatchRenderer;
    GraphicContext m_Context;
    GuiManager m_GuiManager;

    std::vector<Visual*> m_Visuals;
};

ETH_NAMESPACE_END