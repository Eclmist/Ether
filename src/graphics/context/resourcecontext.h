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
#include "graphics/rhi/rhiraytracingpipelinestate.h"
#include "graphics/rhi/rhiaccelerationstructure.h"
#include "graphics/memory/descriptorallocation.h"
#include "graphics/context/graphiccontext.h"

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

    RhiResource& CreateResource(const char* resourceName, const RhiCommitedResourceDesc& desc);
    RhiResource& CreateDepthStencilResource(const char* resourceName, const ethVector2u resolution, RhiFormat format);
    RhiResource& CreateTexture2DResource(const char* resourceName, const ethVector2u resolution, RhiFormat format);
    RhiResource& CreateTexture2DUavResource(const char* resourceName, const ethVector2u resolution, RhiFormat format);
    RhiResource& CreateAccelerationStructure(const char* resourceName, const RhiTopLevelAccelerationStructureDesc& desc);
    RhiResource& CreateRaytracingShaderBindingTable(const char* resourceName, const RhiRaytracingShaderBindingTableDesc& desc);

    RhiRenderTargetView CreateRenderTargetView(const char* viewName, const RhiResource* resource, RhiFormat format);
    RhiDepthStencilView CreateDepthStencilView(const char* viewName, const RhiResource* resource, RhiFormat format);
    RhiConstantBufferView CreateConstantBufferView(const char* viewName, const RhiResource* resource, uint32_t size);
    RhiShaderResourceView CreateShaderResourceView(const char* viewName, const RhiResource* resource, RhiFormat format, RhiShaderResourceDimension dimension);
    RhiUnorderedAccessView CreateUnorderedAccessView(const char* viewName, const RhiResource* resource, RhiFormat format, RhiUnorderedAccessDimension dimension);
    RhiShaderResourceView CreateAccelerationStructureView(const char* viewName, const RhiResource* asDataBufferResource);

    template <typename T>
    T GetResource(const char* resourceName) const;

    template <typename T>
    T GetView(const char* viewName) const;

private:
    bool ShouldRecreateResource(const char* resourceName, const RhiCommitedResourceDesc& desc);
    bool ShouldRecreateResource(const char* resourceName, const RhiRaytracingShaderBindingTableDesc& desc);
    bool ShouldRecreateResource(const char* resourceName, const RhiTopLevelAccelerationStructureDesc& desc);

    bool ShouldRecreateView(const char* viewName);
    void InvalidateViews(const char* resourceName);

private:
    friend class FrameScheduler;
    void RecompilePipelineStates();

private:
    std::unique_ptr<DescriptorAllocator> m_StagingSrvCbvUavAllocator;

    std::unordered_map<RhiPipelineStateDesc*, std::unique_ptr<RhiPipelineState>> m_CachedPipelineStates;

    std::unordered_map<StringID, RhiCommitedResourceDesc> m_ResourceDescriptionTable;
    std::unordered_map<StringID, RhiRaytracingShaderBindingTableDesc> m_RaytracingShaderBindingsTable;
    std::unordered_map<StringID, RhiTopLevelAccelerationStructureDesc> m_RaytracingResourceDescriptionTable;

    std::unordered_map<StringID, std::unique_ptr<RhiResource>> m_ResourceTable;
    std::unordered_map<StringID, std::unique_ptr<RhiResourceView>> m_DescriptorTable;
    std::unordered_map<StringID, std::unique_ptr<MemoryAllocation>> m_DescriptorAllocations;
};

template <typename T>
T Ether::Graphics::ResourceContext::GetResource(const char* resourceName) const
{
    if (m_ResourceTable.find(resourceName) == m_ResourceTable.end())
        LogGraphicsFatal("The requested resource (%s) has not yet been created", resourceName);

    return (T&)*m_ResourceTable.at(resourceName);
}

template <typename T>
T Ether::Graphics::ResourceContext::GetView(const char* viewName) const
{
    if (m_DescriptorTable.find(viewName) == m_DescriptorTable.end())
        LogGraphicsFatal("The requested resource (%s) has not yet been created", viewName);

    return (T&)*m_DescriptorTable.at(viewName);
}

} // namespace Ether::Graphics
