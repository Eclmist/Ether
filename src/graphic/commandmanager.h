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

#include "graphic/commandqueue.h"

ETH_NAMESPACE_BEGIN

class CommandManager : public NonCopyable
{
public:
    void Initialize();
    void Shutdown();

    void CreateCommandList(
        D3D12_COMMAND_LIST_TYPE type,
        ID3D12GraphicsCommandList** cmdList,
        ID3D12CommandAllocator** cmdAlloc);

    void Execute(ID3D12CommandList* cmdLst);
    void Flush();
    ID3D12CommandAllocator* RequestAllocator(D3D12_COMMAND_LIST_TYPE type);
    void DiscardAllocator(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* allocator);

public:
    inline CommandQueue& GetGraphicsQueue() { return *m_GraphicsQueue; }
    inline CommandQueue& GetComputeQueue() { return *m_ComputeQueue; }
    inline CommandQueue& GetCopyQueue() { return *m_CopyQueue; }
    CommandQueue& GetQueue(D3D12_COMMAND_LIST_TYPE type);

private:
    std::unique_ptr<CommandQueue> m_GraphicsQueue;
    std::unique_ptr<CommandQueue> m_ComputeQueue;
    std::unique_ptr<CommandQueue> m_CopyQueue;
};

ETH_NAMESPACE_END
