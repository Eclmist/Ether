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

ETH_NAMESPACE_BEGIN

#define ETH_MAX_NUM_SWAPCHAIN_BUFFERS       3

enum class BufferingMode
{
    BUFFERINGMODE_SINGLE = 1,
    BUFFERINGMODE_DOUBLE = 2,
    BUFFERINGMODE_TRIPLE = 3
};

class GraphicDisplay : public NonCopyable
{
public:
    void Initialize();
    void Shutdown();
    // TODO: Support resize and dynamic update of vsync and buffering mode
    void Present();

public:
    std::shared_ptr<TextureResource> GetCurrentBackBuffer() const;
    std::shared_ptr<DepthStencilResource> GetDepthBuffer() const;

    inline uint64_t GetCurrentBackBufferFence() const { return m_FrameBufferFences[m_CurrentBackBufferIndex]; }
    inline void SetCurrentBackBufferFence(uint64_t fenceValue) { m_FrameBufferFences[m_CurrentBackBufferIndex] = fenceValue; }

    inline bool IsVsyncEnabled() const { return m_VsyncEnabled; }
    inline uint32_t GetNumBuffers() const { return (uint32_t)m_BufferingMode; }

    inline const D3D12_VIEWPORT& GetViewport() const { return m_Viewport; }
    inline const D3D12_RECT& GetScissorRect() const { return m_ScissorRect; }

    inline void SetBufferingMode(BufferingMode mode) { m_BufferingMode = mode; }
    inline void SetVsyncEnabled(bool enabled) { m_VsyncEnabled = enabled; }

private:
    void CreateDxgiSwapChain();
    void InitializeResources();

private:
    wrl::ComPtr<IDXGISwapChain4> m_SwapChain;

    std::shared_ptr<TextureResource> m_FrameBuffers[ETH_MAX_NUM_SWAPCHAIN_BUFFERS];
    std::shared_ptr<DepthStencilResource> m_DepthBuffer;

    uint64_t m_FrameBufferFences[ETH_MAX_NUM_SWAPCHAIN_BUFFERS];

    BufferingMode m_BufferingMode;
    uint32_t m_CurrentBackBufferIndex;

    uint32_t m_FrameBufferWidth;
    uint32_t m_FrameBufferHeight;

    D3D12_RECT m_ScissorRect;
    D3D12_VIEWPORT m_Viewport;

    bool m_VsyncEnabled;
};

ETH_NAMESPACE_END