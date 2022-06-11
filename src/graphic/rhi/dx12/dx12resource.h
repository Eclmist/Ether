/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "graphic/rhi/rhiresource.h"

ETH_NAMESPACE_BEGIN

class Dx12Resource : public RhiResource
{
public:
    Dx12Resource() = default;
    ~Dx12Resource() override = default;

public:
    RhiGpuHandle GetGpuHandle() const override;
    RhiResult SetName(const std::wstring& name) const override;
    RhiResult Map(void** mappedAddr) const override;
    RhiResult Unmap() const override;

private:
    friend class Dx12CommandList;
    friend class Dx12Device;
    friend class Dx12SwapChain;

    wrl::ComPtr<ID3D12Resource> m_Resource;
};

ETH_NAMESPACE_END

