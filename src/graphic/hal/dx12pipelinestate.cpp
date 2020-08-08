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

#include "dx12pipelinestate.h"

DX12PipelineState::DX12PipelineState(
    wrl::ComPtr<ID3D12Device3> device,
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {})
    : m_PipelineStateDesc(psoDesc)
{
    Recompile(device);
}

void DX12PipelineState::Recompile(wrl::ComPtr<ID3D12Device3> device)
{
    ThrowIfFailed(device->CreateGraphicsPipelineState(&m_PipelineStateDesc, IID_PPV_ARGS(&m_PipelineState)));
}
