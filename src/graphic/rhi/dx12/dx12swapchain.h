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

#include "graphic/rhi/rhiswapchain.h"

ETH_NAMESPACE_BEGIN

class Dx12SwapChain : public RhiSwapChain
{
public:
    Dx12SwapChain() = default;
    ~Dx12SwapChain() override = default;

public:
    uint32_t GetCurrentBackBufferIndex() const override;
    RhiResourceHandle GetBuffer(uint8_t index) const override;
    RhiResult ResizeBuffers(RhiResizeDesc& desc) override;
    RhiResult Present(uint8_t numVblanks) override;

private:
    friend class Dx12Device;
    wrl::ComPtr<IDXGISwapChain4> m_SwapChain;
};

ETH_NAMESPACE_END

