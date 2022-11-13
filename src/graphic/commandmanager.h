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

#pragma once

#include "graphic/commandallocatorpool.h"

ETH_NAMESPACE_BEGIN

class CommandManager : public NonCopyable
{
public:
    CommandManager();
    ~CommandManager();

    void CreateCommandList(
        RhiCommandListType type,
        RhiCommandListHandle& cmdList,
        RhiCommandAllocatorHandle& cmdAlloc);

    void Execute(RhiCommandListHandle cmdList);
    void Flush();

public:
    inline RhiCommandQueueHandle GetGraphicsQueue() const { return m_GraphicsQueue; }
    inline RhiCommandQueueHandle GetComputeQueue() const { return m_ComputeQueue; }
    inline RhiCommandQueueHandle GetCopyQueue() const { return m_CopyQueue; }

    inline CommandAllocatorPool& GetGraphicAllocatorPool() const { return *m_GraphicAllocatorPool; }
    inline CommandAllocatorPool& GetComputeAllocatorPool() const { return *m_ComputeAllocatorPool; }
    inline CommandAllocatorPool& GetCopyAllocatorPool() const { return *m_CopyAllocatorPool; }

    RhiCommandQueueHandle GetQueue(RhiCommandListType type) const;
    CommandAllocatorPool& GetAllocatorPool(RhiCommandListType type) const;

private:
    RhiCommandQueueHandle m_GraphicsQueue;
    RhiCommandQueueHandle m_ComputeQueue;
    RhiCommandQueueHandle m_CopyQueue;

    std::unique_ptr<CommandAllocatorPool> m_GraphicAllocatorPool;
    std::unique_ptr<CommandAllocatorPool> m_ComputeAllocatorPool;
    std::unique_ptr<CommandAllocatorPool> m_CopyAllocatorPool;
};

ETH_NAMESPACE_END

