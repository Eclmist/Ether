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

#include "dx12committedresource.h"

DX12CommittedResource::DX12CommittedResource(
    wrl::ComPtr<ID3D12Device3> device,
    size_t resourceSize,
    D3D12_HEAP_TYPE resourceHeapType,
    D3D12_HEAP_FLAGS resourceHeapFlag,
    D3D12_RESOURCE_STATES initialResourceState)
{
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(resourceHeapType),
        resourceHeapFlag,
        &CD3DX12_RESOURCE_DESC::Buffer(resourceSize),
        initialResourceState,
        nullptr,
        IID_PPV_ARGS(&m_Resource)
    ));
}

