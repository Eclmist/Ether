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
#include "graphics/memory/uploadbufferallocator.h"
#include "graphics/rhi/rhicommandlist.h"

namespace Ether::Graphics
{
    class RhiCommandQueue;
    class RhiCommandAllocator;
    class CommandAllocatorPool;

    class ETH_GRAPHIC_DLL CommandContext : public NonCopyable, public NonMovable
    {
    public:
        CommandContext(RhiCommandType type, const std::string& contextName = "Unnamed Command Context");
        ~CommandContext() = default;

        inline RhiCommandList& GetCommandList() const { return *m_CommandList; }

    public:
        void SetMarker(const std::string& name);
        void PushMarker(const std::string& name);
        void PopMarker();

        void TransitionResource(RhiResource& resource, RhiResourceState newState);
        void SetDescriptorHeap(const RhiDescriptorHeap& descriptorHeap);

        void FinalizeAndExecute(bool waitForCompletion = false);

    protected:
        std::string m_Name;
        RhiCommandType m_Type;

        RhiCommandQueue* m_CommandQueue;
        RhiCommandAllocator* m_CommandAllocator;
        CommandAllocatorPool* m_CommandAllocatorPool;

        std::unique_ptr<RhiCommandList> m_CommandList;

        UploadBufferAllocator m_UploadBufferAllocator;
    };
}

