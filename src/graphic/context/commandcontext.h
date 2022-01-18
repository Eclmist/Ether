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

#include "graphic/memory/uploadbufferallocator.h"

ETH_NAMESPACE_BEGIN

class CommandQueue;

class CommandContext : public NonCopyable
{
public:
    CommandContext(RHICommandListType type, const std::wstring& contextName = L"Unammed Command Context");
    ~CommandContext();
    void Reset();

    RHIFenceValue GetCompletionFenceValue() const;

public:
    inline RHICommandListHandle GetCommandList() const { return m_CommandList; }
    inline RHICommandQueueHandle GetCommandQueue() const { return m_CommandQueue; }

public:
    void TransitionResource(RHIResourceHandle resource, RHIResourceState newState);
    void CopyBufferRegion(RHIResourceHandle dest, RHIResourceHandle src, size_t size, size_t dstOffset = 0);
    void FinalizeAndExecute(bool waitForCompletion = false);

public:
    static void InitializeBuffer(RHIResourceHandle dest, const void* data, size_t size, size_t dstOffset = 0);

protected:
    RHICommandAllocatorHandle m_CommandAllocator;
    RHICommandListHandle m_CommandList;
    RHICommandListType m_Type;
    RHICommandQueueHandle m_CommandQueue;

    UploadBufferAllocator m_UploadBufferAllocator;

    std::wstring m_ContextName;
};

ETH_NAMESPACE_END

