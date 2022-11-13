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

#include "graphic/memory/uploadbufferallocator.h"

ETH_NAMESPACE_BEGIN

class CommandQueue;

class CommandContext : public NonCopyable
{
public:
    CommandContext(RhiCommandListType type, const std::wstring& contextName = L"Unnamed Command Context");
    ~CommandContext();
    void Reset();

    RhiFenceValue GetCompletionFenceValue() const;

public:
    inline RhiCommandListHandle GetCommandList() const { return m_CommandList; }
    inline RhiCommandQueueHandle GetCommandQueue() const { return m_CommandQueue; }

public:
    void SetMarker(const std::string& name);
    void PushMarker(const std::string& name);
    void PopMarker();

    void TransitionResource(RhiResourceHandle resource, RhiResourceState newState);
    void CopyBufferRegion(RhiResourceHandle src, RhiResourceHandle dest, size_t size, size_t srcOffset = 0, size_t destOffset = 0);
    void CopyTextureRegion(RhiResourceHandle intermediate, RhiResourceHandle dest, const CompiledTexture& texture, size_t intermediateOffset);
    void InitializeBufferRegion(RhiResourceHandle dest, const void* data, size_t size, size_t destOffset = 0);
    void InitializeTexture(const CompiledTexture& texture);
    void FinalizeAndExecute(bool waitForCompletion = false);

private:
    friend class VisualNode; // TODO: Visual node is the only one initializing without context. remove eventually.
    static void InitializeBufferTemp(RhiResourceHandle dest, const void* data, size_t size, size_t destOffset = 0);

protected:
    RhiCommandAllocatorHandle m_CommandAllocator;
    RhiCommandListHandle m_CommandList;
    RhiCommandListType m_Type;
    RhiCommandQueueHandle m_CommandQueue;

    UploadBufferAllocator m_UploadBufferAllocator;

    std::wstring m_ContextName;
};

ETH_NAMESPACE_END

