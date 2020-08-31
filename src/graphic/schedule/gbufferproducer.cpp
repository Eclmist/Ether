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

#include "gbufferproducer.h"

#include "graphic/hal/dx12device.h"
#include "graphic/hal/dx12swapchain.h"
#include "graphic/gfx/gfxcontext.h"
#include "graphic/gfx/gfxrenderer.h"

GBufferProducer::GBufferProducer()
    : GfxProducer("GBuffer Producer")
{
    //for (int i = 0; i < ETH_NUM_SWAPCHAIN_BUFFERS; ++i)
    //{
    //    m_CommandAllocators[i] = std::make_unique<DX12CommandAllocator>(m_Context->GetDevice()->Get(), D3D12_COMMAND_LIST_TYPE_DIRECT); 
    //    m_FenceValues[i] = 0;
    //}

    //m_CommandList = std::make_unique<DX12CommandList>(m_Context->GetDevice()->Get(), m_CommandAllocators[0]->Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);
}

void GBufferProducer::BuildCommandLists()
{
    GfxGraphicObject* object = m_StaticObjects[0];

    m_Context->GetRenderer()->SetRTCommonParams(*m_CommandList);

    // Root signature must be explicitly set even though it is already set in the PSO
    // for some reason or other
    m_CommandList->Get()->SetGraphicsRootSignature(m_RootSignature.Get());
    m_CommandList->Get()->SetGraphicsRoot32BitConstants(0, sizeof(ethXMMatrix) / 4, &object->GetModelMatrix(), 0);

    // Bind the PSO
    m_CommandList->Get()->SetPipelineState(m_PipelineState->Get().Get());

    // Setup the input assembler
    m_CommandList->Get()->IASetPrimitiveTopology(m_Context->GetRenderWireframe() ? D3D_PRIMITIVE_TOPOLOGY_LINESTRIP : D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_CommandList->Get()->IASetVertexBuffers(0, 1, &object->GetVertexBufferView());
    m_CommandList->Get()->IASetIndexBuffer(&object->GetIndexBufferView());

    // Issue draw call
    m_CommandList->Get()->DrawIndexedInstanced(_countof(g_Indicies), 1, 0, 0, 0);
}

void GBufferProducer::Reset()
{
    uint32_t currentBackBufferIndex = m_Context->GetSwapChain()->GetCurrentBackBufferIndex();

    m_CommandAllocators[currentBackBufferIndex]->Get()->Reset();
    m_CommandList->Get()->Reset(m_CommandAllocators[currentBackBufferIndex]->Get().Get(), nullptr);
}

bool GBufferProducer::GetInputOutput(GfxScheduleContext& scheduleContext)
{
    return true;
}

void GBufferProducer::AddStaticObject(GfxGraphicObject* graphicObject)
{
    m_StaticObjects.push_back(graphicObject);
}

void GBufferProducer::AddDynamicObject(GfxGraphicObject* graphicObject)
{
    throw std::exception("Not yet implemented");
}

