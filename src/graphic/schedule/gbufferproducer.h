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

#include "graphic/gfx/gfxproducer.h"
#include "graphic/gfx/gfxgraphicobject.h"

class GBufferProducer : public GfxProducer
{
public:
    GBufferProducer();
    ~GBufferProducer() = default;

    bool GetInputOutput(GfxScheduleContext& scheduleContext);

    void AddStaticObject(GfxGraphicObject* graphicObject);
    void AddDynamicObject(GfxGraphicObject* graphicObject);

private:
    void BuildCommandLists();
    void Reset();

private:
    std::unique_ptr<DX12CommandList> m_CommandList;
    std::unique_ptr<DX12CommandAllocator> m_CommandAllocators[ETH_NUM_SWAPCHAIN_BUFFERS];
    std::unique_ptr<DX12CommandAllocator> m_CopyCommandAllocators[ETH_NUM_SWAPCHAIN_BUFFERS];
    wrl::ComPtr<ID3D12RootSignature> m_RootSignature; //TODO: Create DX12RootSignature object

    // TODO: Use the resource allocators
    wrl::ComPtr<ID3D12Resource> m_VertexBuffer;
    wrl::ComPtr<ID3D12Resource> m_IntermediateVertexBuffer; // to be used to upload vertex buffer for the first time

    wrl::ComPtr<ID3D12Resource> m_IndexBuffer;
    wrl::ComPtr<ID3D12Resource> m_IntermediateIndexBuffer; // to be used to upload index buffer for the first time

    uint64_t m_FenceValues[ETH_NUM_SWAPCHAIN_BUFFERS];

    std::vector<GfxGraphicObject*> m_StaticObjects;
    std::vector<GfxGraphicObject*> m_DynamicObjects;
};
