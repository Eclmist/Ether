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

CommandManager::CommandManager()
{
    m_GraphicsQueue = std::make_shared<CommandQueue>(D3D12_COMMAND_LIST_TYPE_DIRECT);
    m_ComputeQueue = std::make_shared<CommandQueue>(D3D12_COMMAND_LIST_TYPE_COMPUTE);
    m_CopyQueue = std::make_shared<CommandQueue>(D3D12_COMMAND_LIST_TYPE_COPY);
}

void CommandManager::CreateCommandList(
    D3D12_COMMAND_LIST_TYPE type,
    ID3D12GraphicsCommandList** cmdList,
    ID3D12CommandAllocator** cmdAlloc) const
{
    AssertGraphics(type != D3D12_COMMAND_LIST_TYPE_BUNDLE, "Bundles are not yet supported");
    switch (type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT: *cmdAlloc = m_GraphicsQueue->RequestAllocator(); break;
    case D3D12_COMMAND_LIST_TYPE_COMPUTE: *cmdAlloc = m_ComputeQueue->RequestAllocator(); break;
    case D3D12_COMMAND_LIST_TYPE_COPY: *cmdAlloc = m_CopyQueue->RequestAllocator(); break;
    default:
        LogGraphicsError("Unsupported commandlist type requested");
        return;
    }

    ASSERT_SUCCESS(g_GraphicDevice->CreateCommandList(1, type, *cmdAlloc, nullptr, IID_PPV_ARGS(cmdList)), CommandList);
    (*cmdList)->SetName(L"CommandManager::CommandList");
}

ID3D12CommandQueue* CommandManager::GetQueue(D3D12_COMMAND_LIST_TYPE type) const
{
    switch (type)
    {
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        return m_ComputeQueue->m_CommandQueue.Get();
    case D3D12_COMMAND_LIST_TYPE_COPY:
        return m_CopyQueue->m_CommandQueue.Get();
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
    default:
        return m_GraphicsQueue->m_CommandQueue.Get();
    }
}

ETH_NAMESPACE_END
