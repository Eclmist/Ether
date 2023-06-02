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
#include "graphics/rhi/rhiswapchain.h"
#include "graphics/rhi/dx12/dx12includes.h"

namespace Ether::Graphics
{
class Dx12SwapChain : public RhiSwapChain
{
public:
    Dx12SwapChain();
    ~Dx12SwapChain() override = default;

public:
    uint32_t GetCurrentBackBufferIndex() const override;
    RhiResource& GetBuffer(uint8_t index) const override;
    void ResizeBuffers(const ethVector2u& size) override;
    void ResetBuffers() override;
    void Present(uint8_t numVblanks) override;

private:
    friend class Dx12Device;
    wrl::ComPtr<IDXGISwapChain4> m_SwapChain;
    std::unique_ptr<RhiResource> m_BufferResources[MaxSwapChainBuffers];
};
} // namespace Ether::Graphics
