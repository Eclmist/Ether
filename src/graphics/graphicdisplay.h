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
#include "graphics/memory/descriptorallocator.h"

namespace Ether::Graphics
{
    enum class BufferingMode : unsigned int
    {
        Single = 1,
        Double = 2,
        Triple = 3
    };

    class GraphicDisplay : public NonCopyable, public NonMovable
    {
    public:
        GraphicDisplay();
        ~GraphicDisplay();

    public:
        void Present();
        void ResizeBuffers(const ethVector2u& size);

    public:
        inline uint32_t GetNumBuffers() const { return (uint32_t)m_BufferingMode; }
        inline uint32_t GetBackBufferIndex() const { return m_CurrentBackBufferIndex; }
        inline uint64_t GetBackBufferFence() const { return m_FrameBufferFences[GetBackBufferIndex()]; }
        inline RhiResource& GetBackBuffer() const { return *m_RenderTargets[GetBackBufferIndex()]; }
        inline RhiResource& GetExportBuffer() const { return *m_ExportResource; }
        inline RhiRenderTargetView GetBackBufferRtv() const { return m_RenderTargetRtv[GetBackBufferIndex()]; }
        inline RhiShaderResourceView GetBackBufferSrv() const { return m_RenderTargetSrv[GetBackBufferIndex()]; }
        inline const RhiViewportDesc& GetViewport() const { return m_Viewport; }
        inline const RhiScissorDesc& GetScissorRect() const { return m_ScissorRect; }
        inline bool IsVsyncEnabled() const { return m_VSyncEnabled; }

        inline void SetCurrentBackBufferFence(uint64_t fenceValue) { m_FrameBufferFences[m_CurrentBackBufferIndex] = fenceValue; }
        inline void SetVSyncEnabled(bool enabled) { m_VSyncEnabled = enabled; }
        inline void SetVSyncVBlanks(int numVblanks) { m_VSyncVBlanks = numVblanks; }

    private:
        void InitializeFences();
        void ResizeViewport(const ethVector2u& size);
        void CreateSwapChain(void* hwnd);
        void CreateResourcesFromSwapChain();
        void CreateResourceForExport();
        void CreateViewsFromSwapChain();
        void UpdateBackBufferIndex();

    private:
        std::unique_ptr<RhiSwapChain> m_SwapChain;
        BufferingMode m_BufferingMode;

        RhiResource* m_RenderTargets[MaxSwapChainBuffers];
        RhiRenderTargetView m_RenderTargetRtv[MaxSwapChainBuffers];
        RhiShaderResourceView m_RenderTargetSrv[MaxSwapChainBuffers];

        std::vector<std::unique_ptr<MemoryAllocation>> m_SwapChainDescriptors;
        uint64_t m_FrameBufferFences[MaxSwapChainBuffers];
        uint32_t m_CurrentBackBufferIndex;

        RhiScissorDesc m_ScissorRect;
        RhiViewportDesc m_Viewport;

        bool m_VSyncEnabled;
        uint8_t m_VSyncVBlanks;

        // RtCamp
        std::unique_ptr<RhiResource> m_ExportResource;
    };
}

