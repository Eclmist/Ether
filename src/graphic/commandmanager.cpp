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

#include "commandmanager.h"

ETH_NAMESPACE_BEGIN

void CommandManager::Initialize()
{
    m_GraphicsQueue = std::make_shared<CommandQueue>(D3D12_COMMAND_LIST_TYPE_DIRECT);
    m_ComputeQueue = std::make_shared<CommandQueue>(D3D12_COMMAND_LIST_TYPE_COMPUTE);
    m_CopyQueue = std::make_shared<CommandQueue>(D3D12_COMMAND_LIST_TYPE_COPY);
}

void CommandManager::Shutdown()
{
}

void CommandManager::CreateCommandList(
    D3D12_COMMAND_LIST_TYPE type,
    ID3D12GraphicsCommandList** cmdList,
    ID3D12CommandAllocator** cmdAlloc) const
{
    switch (type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        *cmdAlloc = m_GraphicsQueue->RequestAllocator(); 
        break;
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        *cmdAlloc = m_ComputeQueue->RequestAllocator();
        break;
    case D3D12_COMMAND_LIST_TYPE_COPY:
        *cmdAlloc = m_CopyQueue->RequestAllocator();
        break;
    default:
        LogGraphicsError("Unsupported command list type requested(%s)", type);
        return;
    }

    ASSERT_SUCCESS(g_GraphicDevice->CreateCommandList(1, type, *cmdAlloc, nullptr, IID_PPV_ARGS(cmdList)));
    (*cmdList)->SetName(L"CommandManager::CommandList");
}

void CommandManager::Execute(ID3D12CommandList* cmdLst)
{
    switch (cmdLst->GetType())
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        m_GraphicsQueue->Execute(cmdLst);
        break;
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        m_ComputeQueue->Execute(cmdLst);
        break;
    case D3D12_COMMAND_LIST_TYPE_COPY:
        m_CopyQueue->Execute(cmdLst);
        break;
    default:
        LogGraphicsError("An unsupported command list type (%s) was sent for execution", cmdLst->GetType());
        break;
    }
}

std::shared_ptr<CommandQueue> CommandManager::GetQueue(D3D12_COMMAND_LIST_TYPE type) const
{
    switch (type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        return m_GraphicsQueue;
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        return m_ComputeQueue;
    case D3D12_COMMAND_LIST_TYPE_COPY:
        return m_CopyQueue;
    default:
        LogGraphicsError("An unsupported command list type (%s) was requested", type);
        return m_GraphicsQueue;
        break;
    }
}

ETH_NAMESPACE_END

