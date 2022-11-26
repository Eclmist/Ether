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

#include "graphics/core.h"
#include "graphics/context/commandcontext.h"
#include "graphics/rhi/rhicommandlist.h"
#include "graphics/rhi/rhicommandqueue.h"
#include "graphics/rhi/rhiresource.h"

Ether::Graphics::CommandContext::CommandContext(RhiCommandType type, const std::string& contextName)
    : m_Name(contextName)
    , m_Type(type)
{
    ETH_MARKER_EVENT("Command Context - Constructor");

    m_CommandQueue = &Core::GetCommandManager().GetQueue(type);
    m_CommandAllocatorPool = &Core::GetCommandManager().GetAllocatorPool(type);
    m_CommandAllocator = &m_CommandAllocatorPool->RequestAllocator(m_CommandQueue->GetCurrentFenceValue());
    m_CommandList = Core::GetCommandManager().CreateCommandList(type, *m_CommandAllocator, m_Name + "::CommandList");
}

Ether::Graphics::CommandContext::~CommandContext()
{
    if (m_CommandAllocator != nullptr)
        m_CommandAllocatorPool->DiscardAllocator(*m_CommandAllocator, m_CommandQueue->GetFinalFenceValue());
}

void Ether::Graphics::CommandContext::SetMarker(const std::string& name)
{
    ETH_MARKER_EVENT("Command Context - Set Marker");
    m_CommandList->SetMarker(name);
}

void Ether::Graphics::CommandContext::PushMarker(const std::string& name)
{
    ETH_MARKER_EVENT("Command Context - Push Marker");
    m_CommandList->PushMarker(name);
}

void Ether::Graphics::CommandContext::PopMarker()
{
    ETH_MARKER_EVENT("Command Context - Pop Marker");
    m_CommandList->PopMarker();
}

void Ether::Graphics::CommandContext::TransitionResource(RhiResource& resource, RhiResourceState newState)
{
    ETH_MARKER_EVENT("Command Context - Transition Resource");

    if (resource.GetCurrentState() == newState)
        return;

    RhiResourceTransitionDesc transitionDesc = {};
    transitionDesc.m_Resource = &resource;
    transitionDesc.m_FromState = resource.GetCurrentState();
    transitionDesc.m_ToState = newState;
    m_CommandList->TransitionResource(transitionDesc);
}

void Ether::Graphics::CommandContext::SetDescriptorHeap(const RhiDescriptorHeap& descriptorHeap)
{
    ETH_MARKER_EVENT("Command Context - Set Descriptor Heap");

    const RhiDescriptorHeap* heaps[1] = { &descriptorHeap };

    RhiSetDescriptorHeapsDesc desc;
    desc.m_NumHeaps = 1;
    desc.m_Heaps = heaps;
    m_CommandList->SetDescriptorHeaps(desc);
}

void Ether::Graphics::CommandContext::FinalizeAndExecute(bool waitForCompletion)
{
    ETH_MARKER_EVENT("Command Context - Finalize and Execute");

    Core::GetCommandManager().Execute(*m_CommandList);
    m_CommandAllocatorPool->DiscardAllocator(*m_CommandAllocator, m_CommandQueue->GetFinalFenceValue());

    if (waitForCompletion)
        m_CommandQueue->Flush();
}

void Ether::Graphics::CommandContext::Reset()
{
    m_CommandAllocator = &m_CommandAllocatorPool->RequestAllocator(m_CommandQueue->GetCurrentFenceValue());
    m_CommandList->Reset(*m_CommandAllocator);
}

