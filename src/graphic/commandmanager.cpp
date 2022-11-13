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

#include "commandmanager.h"
#include "graphic/rhi/rhicommandlist.h"
#include "graphic/rhi/rhicommandqueue.h"
#include "graphic/rhi/rhidevice.h"

ETH_NAMESPACE_BEGIN

CommandManager::CommandManager()
{
    m_GraphicsQueue.SetName(L"CommandManager::GraphicsQueue");
    m_ComputeQueue.SetName(L"CommandManager::ComputeQueue");
    m_CopyQueue.SetName(L"CommandManager::CopyQueue");

    GraphicCore::GetDevice()->CreateCommandQueue({ RhiCommandListType::Graphic }, m_GraphicsQueue);
    GraphicCore::GetDevice()->CreateCommandQueue({ RhiCommandListType::Compute }, m_ComputeQueue);
    GraphicCore::GetDevice()->CreateCommandQueue({ RhiCommandListType::Copy }, m_CopyQueue);

    m_GraphicAllocatorPool = std::make_unique<CommandAllocatorPool>(RhiCommandListType::Graphic);
    m_ComputeAllocatorPool = std::make_unique<CommandAllocatorPool>(RhiCommandListType::Compute);
    m_CopyAllocatorPool = std::make_unique<CommandAllocatorPool>(RhiCommandListType::Copy);
}

CommandManager::~CommandManager()
{
    m_GraphicsQueue.Destroy();
    m_ComputeQueue.Destroy();
    m_CopyQueue.Destroy();
}

void CommandManager::CreateCommandList(
    RhiCommandListType type,
    RhiCommandListHandle& cmdList,
    RhiCommandAllocatorHandle& cmdAlloc)
{
    switch (type)
    {
    case RhiCommandListType::Graphic:
        cmdAlloc = m_GraphicAllocatorPool->RequestAllocator(m_GraphicsQueue->GetCompletedFenceValue());
        break;
    case RhiCommandListType::Compute:
        cmdAlloc = m_ComputeAllocatorPool->RequestAllocator(m_ComputeQueue->GetCompletedFenceValue());
        break;
    case RhiCommandListType::Copy:
        cmdAlloc = m_CopyAllocatorPool->RequestAllocator(m_CopyQueue->GetCompletedFenceValue());
        break;
    default:
        LogGraphicsError("Unsupported command list type requested(%s)", type);
        return;
    }

    RhiCommandListDesc desc;
    desc.m_Allocator = cmdAlloc;
    desc.m_Type = type;

    ASSERT_SUCCESS(GraphicCore::GetDevice()->CreateCommandList(desc, cmdList));
}

void CommandManager::Execute(RhiCommandListHandle cmdList)
{
    switch (cmdList->GetType())
    {
    case RhiCommandListType::Graphic:
        m_GraphicsQueue->Execute(cmdList);
        break;
    case RhiCommandListType::Compute:
        m_ComputeQueue->Execute(cmdList);
        break;
    case RhiCommandListType::Copy:
        m_CopyQueue->Execute(cmdList);
        break;
    default:
        LogGraphicsError("An unsupported command list type (%s) was sent for execution", cmdList->GetType());
        break;
    }
}

void CommandManager::Flush()
{
    m_GraphicsQueue->Flush();
    m_ComputeQueue->Flush();
    m_CopyQueue->Flush();
}

RhiCommandQueueHandle CommandManager::GetQueue(RhiCommandListType type) const
{
    switch (type)
    {
    case RhiCommandListType::Graphic:
        return m_GraphicsQueue;
    case RhiCommandListType::Compute:
        return m_ComputeQueue;
    case RhiCommandListType::Copy:
        return m_CopyQueue;
    default:
        LogGraphicsError("An unsupported command list type (%s) was requested", type);
        return m_GraphicsQueue;
        break;
    }
}

CommandAllocatorPool& CommandManager::GetAllocatorPool(RhiCommandListType type) const
{
    switch (type)
    {
    case RhiCommandListType::Graphic:
        return *m_GraphicAllocatorPool;
    case RhiCommandListType::Compute:
        return *m_ComputeAllocatorPool;
    case RhiCommandListType::Copy:
        return *m_CopyAllocatorPool;
    default:
        LogGraphicsError("An unsupported command list type (%s) was requested", type);
        return *m_GraphicAllocatorPool;
        break;
    }
}

ETH_NAMESPACE_END

