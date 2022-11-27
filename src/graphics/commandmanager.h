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

#include "graphics/pch.h"
#include "graphics/commandallocatorpool.h"
#include "graphics/rhi/rhicommandqueue.h"

namespace Ether::Graphics
{
    class CommandManager : public NonCopyable, public NonMovable
    {
    public:
        CommandManager();
        ~CommandManager() = default;

        std::unique_ptr<RhiCommandList> CreateCommandList(RhiCommandType type, RhiCommandAllocator& alloc, const std::string& name);

    public:
        RhiFenceValue Execute(RhiCommandList& cmdList);
        void Flush();

    public:
        inline RhiCommandQueue& GetGraphicQueue() { return *m_GraphicQueue; }
        inline RhiCommandQueue& GetComputeQueue() { return *m_ComputeQueue; }
        inline RhiCommandQueue& GetCopyQueue() { return *m_CopyQueue; }

        inline CommandAllocatorPool& GetGraphicAllocatorPool() { return m_GraphicAllocatorPool; }
        inline CommandAllocatorPool& GetComputeAllocatorPool() { return m_ComputeAllocatorPool; }
        inline CommandAllocatorPool& GetCopyAllocatorPool() { return m_CopyAllocatorPool; }

        RhiCommandQueue& GetQueue(RhiCommandType type);
        CommandAllocatorPool& GetAllocatorPool(RhiCommandType type);

    private:
        std::unique_ptr<RhiCommandQueue> m_GraphicQueue;
        std::unique_ptr<RhiCommandQueue> m_ComputeQueue;
        std::unique_ptr<RhiCommandQueue> m_CopyQueue;

        CommandAllocatorPool m_GraphicAllocatorPool;
        CommandAllocatorPool m_ComputeAllocatorPool;
        CommandAllocatorPool m_CopyAllocatorPool;
    };
}

