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

ETH_NAMESPACE_BEGIN

constexpr uint32_t MaxSwapChainBuffers = 3;
constexpr RhiFormat BackBufferFormat = RhiFormat::R8G8B8A8Unorm;

enum class BufferingMode : unsigned int
{
    Single = 1,
    Double = 2,
    Triple = 3
};

class GraphicDisplay : public NonCopyable
{
public:
    GraphicDisplay();
    ~GraphicDisplay();

    void Present();
    void ResizeBuffers();

public:
    inline uint64_t GetCurrentBackBufferFence() const { return m_FrameBufferFences[m_CurrentBackBufferIndex]; }
    inline void SetCurrentBackBufferFence(uint64_t fenceValue) { m_FrameBufferFences[m_CurrentBackBufferIndex] = fenceValue; }

    inline bool IsVsyncEnabled() const { return m_VSyncEnabled; }
    inline uint32_t GetNumBuffers() const { return (uint32_t)m_BufferingMode; }

    inline const RhiViewportDesc& GetViewport() const { return m_Viewport; }
    inline const RhiScissorDesc& GetScissorRect() const { return m_ScissorRect; }

    inline void SetVSyncEnabled(bool enabled) { m_VSyncEnabled = enabled; }
    inline void SetVSyncVBlanks(int numVblanks) { m_VSyncVBlanks = numVblanks; }

public:
    RhiResourceHandle GetCurrentBackBuffer() const;
    RhiRenderTargetViewHandle GetCurrentBackBufferRtv() const;
    RhiShaderResourceViewHandle GetCurrentBackBufferSrv() const;
    void QueueBufferResize(uint32_t width, uint32_t height);

private:
    void CreateSwapChain();
    void CreateResourcesFromSwapChain();
    void CreateViewsFromSwapChain();
    void ResetCurrentBufferIndex();
    void ResetFences();

private:
    RhiSwapChainHandle m_SwapChain;
    const BufferingMode m_BufferingMode;

    RhiResourceHandle m_RenderTargets[MaxSwapChainBuffers];
    RhiRenderTargetViewHandle m_RenderTargetCpuDescriptors[MaxSwapChainBuffers];
    RhiShaderResourceViewHandle m_RenderTargetGpuDescriptors[MaxSwapChainBuffers];
    
    DescriptorAllocator m_RtvAllocator;

    uint64_t m_FrameBufferFences[MaxSwapChainBuffers];

    uint32_t m_CurrentBackBufferIndex;
    uint32_t m_FrameBufferWidth;
    uint32_t m_FrameBufferHeight;

    RhiScissorDesc m_ScissorRect;
    RhiViewportDesc m_Viewport;

    bool m_VSyncEnabled;
    uint8_t m_VSyncVBlanks;

    std::mutex m_WindowsResizeMutex;
    bool m_ShouldResize;
};

ETH_NAMESPACE_END

