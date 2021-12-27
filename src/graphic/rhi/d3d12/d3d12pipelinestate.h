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

#include "graphic/rhi/rhipipelinestate.h"

ETH_NAMESPACE_BEGIN

class D3D12PipelineState : public RHIPipelineState
{
public:
    D3D12PipelineState() = default;
    ~D3D12PipelineState() = default;

private:
    friend class D3D12CommandList;
    friend class D3D12Device;
    wrl::ComPtr<ID3D12PipelineState> m_PipelineState;
    std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputElements;
};

ETH_NAMESPACE_END

