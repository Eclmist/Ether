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
        ID3D12CommandAllocator** cmdAlloc) const;

    void Execute(ID3D12CommandList* cmdLst);

public:
    inline std::shared_ptr<CommandQueue> GetGraphicsQueue() const { return m_GraphicsQueue; };
    inline std::shared_ptr<CommandQueue> GetComputeQueue() const { return m_ComputeQueue; };
    inline std::shared_ptr<CommandQueue> GetCopyQueue() const { return m_CopyQueue; };
    inline std::shared_ptr<CommandQueue> GetQueue(D3D12_COMMAND_LIST_TYPE type) const;

private:
    std::shared_ptr<CommandQueue> m_GraphicsQueue;
    std::shared_ptr<CommandQueue> m_ComputeQueue;
    std::shared_ptr<CommandQueue> m_CopyQueue;
};

ETH_NAMESPACE_END
