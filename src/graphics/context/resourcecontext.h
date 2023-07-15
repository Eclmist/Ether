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
#include "graphics/rhi/rhitypes.h"
#include "graphics/rhi/rhipipelinestate.h"
#include "graphics/rhi/rhigraphicpipelinestate.h"
#include "graphics/rhi/rhicomputepipelinestate.h"
#include "graphics/rhi/rhiraytracingpipelinestate.h"
#include "graphics/rhi/rhiaccelerationstructure.h"
#include "graphics/memory/descriptorallocator.h"
#include "graphics/memory/descriptorallocation.h"
#include "graphics/context/graphiccontext.h"
#include "graphics/schedule/frameschedulerutils.h"

namespace Ether::Graphics
{
class ResourceContext
{
public:
    ResourceContext();
    ~ResourceContext() = default;

public:
    void RegisterPipelineState(const char* name, RhiPipelineStateDesc& pipelineStateDesc);
    RhiPipelineState& GetPipelineState(RhiPipelineStateDesc& pipelineStateDesc);

    RhiResource& CreateBufferResource(const char* resourceName, size_t size, RhiResourceFlag flags);
    RhiResource& CreateTexture2DResource(const char* resourceName, const ethVector2u resolution, RhiFormat format, RhiResourceFlag flags);
    RhiResource& CreateTexture3DResource(const char* resourceName, const ethVector3u resolution, RhiFormat format, RhiResourceFlag flags);
    RhiResource& CreateAccelerationStructure(const char* resourceName, const RhiTopLevelAccelerationStructureDesc& desc);
    RhiResource& CreateRaytracingShaderBindingTable(const char* resourceName, const RhiRaytracingShaderBindingTableDesc& desc);

    void InitializeRenderTargetView(std::shared_ptr<RhiResourceView> view);
    void InitializeDepthStencilView(std::shared_ptr<RhiResourceView> view);
    void InitializeShaderResourceView(std::shared_ptr<RhiResourceView> view);
    void InitializeUnorderedAccessView(std::shared_ptr<RhiResourceView> view);
    void InitializeConstantBufferView(std::shared_ptr<RhiResourceView> view);

    template <typename T>
    RhiResource* GetResource(GFX_STATIC::StaticResourceWrapper<T> view) const;

private:
    bool ShouldRecreateResource(StringID resourceID, const RhiCommitedResourceDesc& desc);
    bool ShouldRecreateResource(StringID resourceID, const RhiRaytracingShaderBindingTableDesc& desc);
    bool ShouldRecreateResource(StringID resourceID, const RhiTopLevelAccelerationStructureDesc& desc);

    bool ShouldRecreateView(StringID viewID);
    void InvalidateViews(StringID resourceID);
    void InvalidateResource(StringID resourceID);

private:
    friend class FrameScheduler;
    void Reset();

private:
    std::unique_ptr<DescriptorAllocator> m_StagingSrvCbvUavAllocator;

    std::unordered_map<RhiPipelineStateDesc*, std::unique_ptr<RhiPipelineState>> m_CachedPipelineStates;

    std::unordered_map<StringID, RhiCommitedResourceDesc> m_ResourceDescriptionTable;
    std::unordered_map<StringID, RhiRaytracingShaderBindingTableDesc> m_RaytracingShaderBindingsTable;
    std::unordered_map<StringID, RhiTopLevelAccelerationStructureDesc> m_RaytracingResourceDescriptionTable;

    std::unordered_map<StringID, std::unique_ptr<RhiResource>> m_ResourceTable;
    std::unordered_map<StringID, std::shared_ptr<RhiResourceView>> m_DescriptorTable;
    std::unordered_map<StringID, std::unique_ptr<MemoryAllocation>> m_DescriptorAllocations;

    std::queue<std::unique_ptr<RhiResource>> m_StaleResources;
};

template <typename T>
RhiResource* Ether::Graphics::ResourceContext::GetResource(GFX_STATIC::StaticResourceWrapper<T> view) const
{
    if (m_ResourceTable.find(view.GetSharedResourceName()) == m_ResourceTable.end())
        LogGraphicsFatal("The requested resource (%s) has not yet been created", view.GetSharedResourceName());

    return m_ResourceTable.at(view.GetSharedResourceName()).get();
}

} // namespace Ether::Graphics
