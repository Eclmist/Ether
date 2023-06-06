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

#include "graphics/graphiccore.h"
#include "graphics/command/commandmanager.h"
#include "graphics/rhi/rhicommandlist.h"
#include "graphics/rhi/rhicommandqueue.h"
#include "graphics/rhi/rhidevice.h"

Ether::Graphics::CommandManager::CommandManager()
    : m_GraphicAllocatorPool(RhiCommandType::Graphic)
    , m_ComputeAllocatorPool(RhiCommandType::Compute)
    , m_CopyAllocatorPool(RhiCommandType::Copy)
{
    m_GraphicQueue = GraphicCore::GetDevice().CreateCommandQueue({ RhiCommandType::Graphic });
    m_ComputeQueue = GraphicCore::GetDevice().CreateCommandQueue({ RhiCommandType::Compute });
    m_CopyQueue = GraphicCore::GetDevice().CreateCommandQueue({ RhiCommandType::Copy });
}

std::unique_ptr<Ether::Graphics::RhiCommandList> Ether::Graphics::CommandManager::CreateCommandList(
    const char* name,
    RhiCommandType type)
{
    return GraphicCore::GetDevice().CreateCommandList(name, type);
}

Ether::Graphics::RhiFenceValue Ether::Graphics::CommandManager::Execute(RhiCommandList& cmdList)
{
    switch (cmdList.GetType())
    {
    case RhiCommandType::Graphic:
        return m_GraphicQueue->Execute(cmdList);
    case RhiCommandType::Compute:
        return m_ComputeQueue->Execute(cmdList);
    case RhiCommandType::Copy:
        return m_CopyQueue->Execute(cmdList);
    default:
        LogGraphicsError("An unsupported command list type (%s) was sent for execution", cmdList.GetType());
        return m_GraphicQueue->Execute(cmdList);
    }
}

void Ether::Graphics::CommandManager::Flush()
{
    m_GraphicQueue->Flush();
    m_ComputeQueue->Flush();
    m_CopyQueue->Flush();
}

Ether::Graphics::RhiCommandQueue& Ether::Graphics::CommandManager::GetQueue(RhiCommandType type)
{
    switch (type)
    {
    case RhiCommandType::Graphic:
        return *m_GraphicQueue;
    case RhiCommandType::Compute:
        return *m_ComputeQueue;
    case RhiCommandType::Copy:
        return *m_CopyQueue;
    default:
        LogGraphicsError("An unsupported command list type (%s) was requested", type);
        return *m_GraphicQueue;
        break;
    }
}

Ether::Graphics::CommandAllocatorPool& Ether::Graphics::CommandManager::GetAllocatorPool(RhiCommandType type)
{
    switch (type)
    {
    case RhiCommandType::Graphic:
        return m_GraphicAllocatorPool;
    case RhiCommandType::Compute:
        return m_ComputeAllocatorPool;
    case RhiCommandType::Copy:
        return m_CopyAllocatorPool;
    default:
        LogGraphicsError("An unsupported command list type (%s) was requested", type);
        return m_GraphicAllocatorPool;
    }
}

