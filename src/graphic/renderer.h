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

class Renderer : public Subsystem
{
public:
    void Initialize() override;
    void Shutdown() override;

    void Render();

private:
    void InitializeDevice();
    void InitializeSwapChain();

private:
    wrl::ComPtr<IDXGISwapChain4> m_SwapChain;
    wrl::ComPtr<ID3D12Resource> m_FrameBuffers[ETH_NUM_SWAPCHAIN_BUFFERS];

    uint32_t m_FrameBufferWidth;
    uint32_t m_FrameBufferHeight;
    uint32_t m_BackBufferIndex;
};

ETH_NAMESPACE_END