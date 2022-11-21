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
//
//ETH_NAMESPACE_BEGIN
//
//class GraphicRenderer : public NonCopyable, public NonMovable
//{
//public:
//    GraphicRenderer();
//    ~GraphicRenderer();
//
//    void WaitForPresent();
//    void Render();
//    void Present();
//    void CleanUp();
//
//    inline GraphicContext& GetGraphicContext() { return m_GraphicContext; }
//    inline ResourceContext& GetResourceContext() { return m_ResourceContext; }
//
//public:
//    void DrawNode(VisualNode*); // TODO: Move this responsibility into the various render passes
//
//private:
//    GraphicContext m_GraphicContext;
//    ResourceContext m_ResourceContext;
//
//    GraphicScheduler m_Scheduler;
//
//public: 
//    // TODO: Move into some kind of geometry render pass. Right now accessed by HardCodedRenderPass as a POC
//    std::vector<VisualNode*> m_PendingVisualNodes;
//    // TODO: Move this somewhere sane
//    std::shared_ptr<CompiledTexture> m_EnvironmentHDRI;
//};
//
//ETH_NAMESPACE_END

