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

#include "graphic/resource/gpuresource.h"
#include "graphic/resource/textureresource.h"

ETH_NAMESPACE_BEGIN

class CommandQueue;

/*
* A graphic context contains all the information to successfully populate (and
* send for execution) a single command list. In other words, there should be one
* context for each rendering thread (one for UI, one for the main rendering thread,
* one for compute, etc.). 
*/
class GraphicContext : NonCopyable
{
public:
    void Initialize(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);
    void Shutdown();
    void Reset();

public:
    inline ID3D12GraphicsCommandList* GetCommandList() const { return m_CommandList.Get(); }
    inline std::shared_ptr<CommandQueue> GetCommandQueue() const { return m_CommandQueue; }

public:
    // TODO: Create resource class that keeps track of the before states
    void ClearColor(TextureResource& texture, ethVector4 color);
    void TransitionResource(GPUResource& resource, D3D12_RESOURCE_STATES newState);

    void FinalizeAndExecute();

private:
    // One descriptor heap is certainly not enough for actual rendering. This is why
    // most D3D12 apps create some kind of allocator to get just enough descriptor heaps.
    // TODO: Handle descriptor heap allocation once we're doing more than just clearing
    // the screen.
    ID3D12DescriptorHeap* m_CurrentDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    std::shared_ptr<CommandQueue> m_CommandQueue;

    wrl::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
    wrl::ComPtr<ID3D12CommandAllocator> m_CommandAllocator;

    D3D12_COMMAND_LIST_TYPE m_Type;
};

ETH_NAMESPACE_END
