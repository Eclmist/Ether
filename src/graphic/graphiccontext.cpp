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

#include "graphiccontext.h"

ETH_NAMESPACE_BEGIN

void GraphicContext::Initialize(D3D12_COMMAND_LIST_TYPE type)
{
    g_CommandManager.CreateCommandList(type, &m_CommandList, &m_CommandAllocator);
    m_CommandQueue = g_CommandManager.GetQueue(type);
    m_Type = type;
}

void GraphicContext::Shutdown()
{
}

void GraphicContext::Reset()
{
    m_CommandAllocator = g_CommandManager.GetQueue(m_Type)->RequestAllocator();
    m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);
}

void GraphicContext::ClearColor(TextureResource& texture, ethVector4 color)
{
    float clearColor[] = { color.x, color.y, color.z, color.w };
    m_CommandList->ClearRenderTargetView(texture.GetRTV(), clearColor, 0, nullptr);
}

void GraphicContext::TransitionResource(GPUResource& target, D3D12_RESOURCE_STATES newState)
{
    if (target.GetCurrentState() == newState)
        return;

    target.SetNextState(newState);

    D3D12_RESOURCE_BARRIER barrier;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = target.GetResource();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = target.GetCurrentState();
    barrier.Transition.StateAfter = newState;

    m_CommandList->ResourceBarrier(1, &barrier);
    
    // Ideally, this should be called only after checking with a fence that
    // the resource has actually transitioned. (TODO)
    target.TransitionToNextState();
}


void GraphicContext::SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE rtv)
{
    m_CommandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
}

void GraphicContext::FinalizeAndExecute()
{
    g_CommandManager.Execute(m_CommandList.Get());
    m_CommandQueue->DiscardAllocator(m_CommandAllocator.Get(), m_CommandQueue->GetCompletionFence());
}

ETH_NAMESPACE_END

