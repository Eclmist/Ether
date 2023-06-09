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

#include "graphics/context/resourcecontext.h"

#include "graphics/graphiccore.h"
#include "graphics/rhi/rhiresource.h"
#include "graphics/rhi/rhiresourceviews.h"

Ether::Graphics::ResourceContext::ResourceContext()
{
    m_StagingSrvCbvUavAllocator = std::make_unique<DescriptorAllocator>(RhiDescriptorHeapType::SrvCbvUav, _64KiB, false);
}

void Ether::Graphics::ResourceContext::RegisterPipelineState(const char* name, RhiPipelineStateDesc& pipelineStateDesc)
{
    // This caching doesn't actually work since it just cheats by using the address as a key
    // A mechanism for hashing the pipeline state data (maybe inherit serializable and create a stringstream?)
    // (TODO)
    // Note: Shader recompile works by calling this function again for all registered psos.
    // If we replace this function with more complex checking with serialization, this process will be
    // really bad for performance because most PSOs probably don't need shader recompilation every frame,
    // but their hashes will have to be checked and recomputed each time (how expensive would hashing be?
    // maybe we want to rehash every frame anyway just in case something changes, but print a warning
    // RecompilePipelineStates needs to be reworked when this happens.

    // Shaders that require recompilation defintely needs PSOs to be recompiled.
    if (pipelineStateDesc.RequiresShaderCompilation())
    {
        GraphicCore::FlushGpu();
        pipelineStateDesc.CompileShaders();
        m_CachedPipelineStates[&pipelineStateDesc] = pipelineStateDesc.Compile(name);
        return;
    }

    if (m_CachedPipelineStates.find(&pipelineStateDesc) == m_CachedPipelineStates.end())
        m_CachedPipelineStates[&pipelineStateDesc] = pipelineStateDesc.Compile(name);
}

Ether::Graphics::RhiPipelineState& Ether::Graphics::ResourceContext::GetPipelineState(
    RhiPipelineStateDesc& pipelineStateDesc)
{
    if (m_CachedPipelineStates.find(&pipelineStateDesc) == m_CachedPipelineStates.end())
    {
        LogGraphicsError("A pipeline state desc was registered before use");
        RegisterPipelineState("Unknown Pipeline State", pipelineStateDesc);
    }

    return *m_CachedPipelineStates.at(&pipelineStateDesc);
}

Ether::Graphics::RhiResource& Ether::Graphics::ResourceContext::CreateResource(
    const char* resourceName,
    const RhiCommitedResourceDesc& desc)
{
    if (!ShouldRecreateResource(resourceName, desc))
        return *m_ResourceTable.at(resourceName);

    InvalidateViews(resourceName);
    m_ResourceTable[resourceName] = GraphicCore::GetDevice().CreateCommittedResource(desc);
    m_ResourceDescriptionTable[resourceName] = desc;

    return *m_ResourceTable.at(resourceName);
}

Ether::Graphics::RhiResource& Ether::Graphics::ResourceContext::CreateDepthStencilResource(
    const char* resourceName,
    const ethVector2u resolution,
    RhiFormat format)
{
    RhiCommitedResourceDesc desc = {};
    desc.m_Name = resourceName;
    desc.m_HeapType = RhiHeapType::Default;
    desc.m_State = RhiResourceState::DepthWrite;
    desc.m_ClearValue = { format, { 1, 0 } };
    desc.m_ResourceDesc = RhiCreateDepthStencilResourceDesc(DepthBufferFormat, resolution);

    if (!ShouldRecreateResource(resourceName, desc))
        return *m_ResourceTable.at(resourceName);

    InvalidateViews(resourceName);
    m_ResourceTable[resourceName] = GraphicCore::GetDevice().CreateCommittedResource(desc);
    m_ResourceDescriptionTable[resourceName] = desc;

    return *m_ResourceTable.at(resourceName);
}

Ether::Graphics::RhiResource& Ether::Graphics::ResourceContext::CreateTexture2DResource(
    const char* resourceName,
    const ethVector2u resolution,
    RhiFormat format)
{
    RhiCommitedResourceDesc desc = {};
    desc.m_Name = resourceName;
    desc.m_HeapType = RhiHeapType::Default;
    desc.m_State = RhiResourceState::Common;
    desc.m_ClearValue = { format, { 0, 0, 0, 0 } };
    desc.m_ResourceDesc = RhiCreateTexture2DResourceDesc(format, resolution);

    if (!ShouldRecreateResource(resourceName, desc))
        return *m_ResourceTable.at(resourceName);

    InvalidateViews(resourceName);
    m_ResourceTable[resourceName] = GraphicCore::GetDevice().CreateCommittedResource(desc);
    m_ResourceDescriptionTable[resourceName] = desc;

    return *m_ResourceTable.at(resourceName);
}

Ether::Graphics::RhiResource& Ether::Graphics::ResourceContext::CreateTexture2DUavResource(
    const char* resourceName,
    const ethVector2u resolution,
    RhiFormat format)
{ 
    RhiCommitedResourceDesc desc = {};
    desc.m_Name = resourceName;
    desc.m_HeapType = RhiHeapType::Default;
    desc.m_State = RhiResourceState::Common;
    desc.m_ClearValue = { format, { 0, 0, 0, 0 } };
    desc.m_ResourceDesc = RhiCreateTexture2DResourceDesc(format, resolution);
    desc.m_ResourceDesc.m_Flag = RhiResourceFlag::AllowUnorderedAccess;

    if (!ShouldRecreateResource(resourceName, desc))
        return *m_ResourceTable.at(resourceName);

    InvalidateViews(resourceName);
    m_ResourceTable[resourceName] = GraphicCore::GetDevice().CreateCommittedResource(desc);
    m_ResourceDescriptionTable[resourceName] = desc;

    return *m_ResourceTable.at(resourceName);
}

Ether::Graphics::RhiResource& Ether::Graphics::ResourceContext::CreateAccelerationStructure(
    const char* resourceName,
    const RhiTopLevelAccelerationStructureDesc& desc)
{
    if (!ShouldRecreateResource(resourceName, desc))
        return *m_ResourceTable.at(resourceName);

    InvalidateViews(resourceName);

    std::unique_ptr<RhiAccelerationStructure> as = GraphicCore::GetDevice().CreateAccelerationStructure(desc);

    CommandContext ctx("CommandContext - Build TLAS");
    ctx.Reset();
    ctx.TransitionResource(*as->m_ScratchBuffer, RhiResourceState::UnorderedAccess);
    ctx.BuildTopLevelAccelerationStructure(*as);
    ctx.FinalizeAndExecute(true);

    m_ResourceTable[resourceName] = std::move(as->m_DataBuffer);
    m_RaytracingResourceDescriptionTable[resourceName] = desc;

    return *m_ResourceTable.at(resourceName);
}

Ether::Graphics::RhiResource& Ether::Graphics::ResourceContext::CreateRaytracingShaderBindingTable(
    const char* resourceName,
    const RhiRaytracingShaderBindingTableDesc& desc)
{
    if (!ShouldRecreateResource(resourceName, desc))
        return *m_ResourceTable.at(resourceName);

    InvalidateViews(resourceName);

    m_ResourceTable[resourceName] = GraphicCore::GetDevice().CreateRaytracingShaderBindingTable(resourceName, desc);
    m_RaytracingShaderBindingsTable[resourceName] = desc;

    return *m_ResourceTable.at(resourceName);
}

Ether::Graphics::RhiRenderTargetView Ether::Graphics::ResourceContext::CreateRenderTargetView(
    const char* viewName,
    const RhiResource* resource,
    RhiFormat format)
{
    if (!ShouldRecreateView(viewName))
        return (RhiRenderTargetView&)(*m_DescriptorTable.at(viewName));

    auto alloc = GraphicCore::GetRtvAllocator().Allocate();

    RhiRenderTargetViewDesc rtvDesc = {};
    rtvDesc.m_Format = format;
    rtvDesc.m_Resource = const_cast<RhiResource*>(resource);
    rtvDesc.m_TargetCpuAddress = ((DescriptorAllocation&)(*alloc)).GetCpuAddress();

    m_DescriptorTable[viewName] = GraphicCore::GetDevice().CreateRenderTargetView(rtvDesc);
    m_DescriptorAllocations[viewName] = std::move(alloc);

    return (RhiRenderTargetView&)(*m_DescriptorTable.at(viewName));
}

Ether::Graphics::RhiDepthStencilView Ether::Graphics::ResourceContext::CreateDepthStencilView(
    const char* viewName,
    const RhiResource* resource,
    RhiFormat format)
{
    if (!ShouldRecreateView(viewName))
        return (RhiDepthStencilView&)(*m_DescriptorTable.at(viewName));

    auto alloc = GraphicCore::GetDsvAllocator().Allocate();

    RhiDepthStencilViewDesc dsvDesc = {};
    dsvDesc.m_Format = format;
    dsvDesc.m_Resource = const_cast<RhiResource*>(resource);
    dsvDesc.m_TargetCpuAddress = ((DescriptorAllocation&)(*alloc)).GetCpuAddress();

    m_DescriptorTable[viewName] = GraphicCore::GetDevice().CreateDepthStencilView(dsvDesc);
    m_DescriptorAllocations[viewName] = std::move(alloc);

    return (RhiDepthStencilView&)(*m_DescriptorTable.at(viewName));
}

Ether::Graphics::RhiConstantBufferView Ether::Graphics::ResourceContext::CreateConstantBufferView(
    const char* viewName,
    const RhiResource* resource,
    uint32_t size)
{
    if (!ShouldRecreateView(viewName))
        return (RhiConstantBufferView&)(*m_DescriptorTable.at(viewName));

    auto alloc = m_StagingSrvCbvUavAllocator->Allocate();

    RhiConstantBufferViewDesc cbvDesc = {};
    cbvDesc.m_Resource = const_cast<RhiResource*>(resource);
    cbvDesc.m_BufferSize = size;
    cbvDesc.m_TargetCpuAddress = ((DescriptorAllocation&)(*alloc)).GetCpuAddress();
    cbvDesc.m_TargetGpuAddress = ((DescriptorAllocation&)(*alloc)).GetGpuAddress();

    m_DescriptorTable[viewName] = GraphicCore::GetDevice().CreateConstantBufferView(cbvDesc);
    m_DescriptorAllocations[viewName] = std::move(alloc);

    return (RhiConstantBufferView&)(*m_DescriptorTable.at(viewName));
}

Ether::Graphics::RhiShaderResourceView Ether::Graphics::ResourceContext::CreateShaderResourceView(
    const char* viewName,
    const RhiResource* resource,
    RhiFormat format,
    RhiShaderResourceDimension dimension)
{
    if (!ShouldRecreateView(viewName))
        return (RhiShaderResourceView&)(*m_DescriptorTable.at(viewName));

    auto alloc = m_StagingSrvCbvUavAllocator->Allocate();

    RhiShaderResourceViewDesc srvDesc = {};
    srvDesc.m_Resource = const_cast<RhiResource*>(resource);
    srvDesc.m_Format = format;
    srvDesc.m_Dimensions = dimension;
    srvDesc.m_TargetCpuAddress = ((DescriptorAllocation&)(*alloc)).GetCpuAddress();
    srvDesc.m_TargetGpuAddress = ((DescriptorAllocation&)(*alloc)).GetGpuAddress();

    m_DescriptorTable[viewName] = GraphicCore::GetDevice().CreateShaderResourceView(srvDesc);
    m_DescriptorAllocations[viewName] = std::move(alloc);

    return (RhiShaderResourceView&)(*m_DescriptorTable.at(viewName));
}

Ether::Graphics::RhiUnorderedAccessView Ether::Graphics::ResourceContext::CreateUnorderedAccessView(
    const char* viewName,
    const RhiResource* resource,
    RhiFormat format,
    RhiUnorderedAccessDimension dimension)
{
    if (!ShouldRecreateView(viewName))
        return (RhiUnorderedAccessView&)(*m_DescriptorTable.at(viewName));

    auto alloc = m_StagingSrvCbvUavAllocator->Allocate();

    RhiUnorderedAccessViewDesc uavDesc = {};
    uavDesc.m_Resource = const_cast<RhiResource*>(resource);
    uavDesc.m_Format = format;
    uavDesc.m_Dimensions = dimension;
    uavDesc.m_TargetCpuAddress = ((DescriptorAllocation&)(*alloc)).GetCpuAddress();
    uavDesc.m_TargetGpuAddress = ((DescriptorAllocation&)(*alloc)).GetGpuAddress();

    m_DescriptorTable[viewName] = GraphicCore::GetDevice().CreateUnorderedAccessView(uavDesc);
    m_DescriptorAllocations.erase(viewName);
    m_DescriptorAllocations[viewName] = std::move(alloc);

    return (RhiUnorderedAccessView&)(*m_DescriptorTable.at(viewName));
}

Ether::Graphics::RhiShaderResourceView Ether::Graphics::ResourceContext::CreateAccelerationStructureView(
    const char* viewName,
    const RhiResource* asDataBufferResource)
{
    if (!ShouldRecreateView(viewName))
        return (RhiShaderResourceView&)(*m_DescriptorTable.at(viewName));

    auto alloc = m_StagingSrvCbvUavAllocator->Allocate();

    RhiShaderResourceViewDesc srvDesc = {};
    srvDesc.m_Resource = const_cast<RhiResource*>(asDataBufferResource);
    srvDesc.m_Dimensions = RhiShaderResourceDimension::RTAccelerationStructure;
    srvDesc.m_TargetCpuAddress = ((DescriptorAllocation&)(*alloc)).GetCpuAddress();
    srvDesc.m_TargetGpuAddress = ((DescriptorAllocation&)(*alloc)).GetGpuAddress();

    m_DescriptorTable[viewName] = GraphicCore::GetDevice().CreateShaderResourceView(srvDesc);
    m_DescriptorAllocations[viewName] = std::move(alloc);

    return (RhiShaderResourceView&)(*m_DescriptorTable.at(viewName));
}

bool Ether::Graphics::ResourceContext::ShouldRecreateResource(
    const char* resourceName,
    const RhiCommitedResourceDesc& desc)
{
    // If the resource don't exist in the resource table at all
    if (m_ResourceTable.find(resourceName) == m_ResourceTable.end())
        return true;

    AssertGraphics(
        m_ResourceDescriptionTable.find(resourceName) != m_ResourceDescriptionTable.end(),
        "If the resource never existed, there should not be any cached desc with the same resourceName");

    // If the resource exist, but it's description has changed
    if (std::memcmp(
            &m_ResourceDescriptionTable.at(resourceName).m_ResourceDesc,
            &desc.m_ResourceDesc,
            sizeof(RhiResourceDesc)) != 0)
        return true;

    return false;
}

bool Ether::Graphics::ResourceContext::ShouldRecreateResource(
    const char* resourceName,
    const RhiTopLevelAccelerationStructureDesc& desc)
{
    // If the resource don't exist in the resource table at all
    if (m_ResourceTable.find(resourceName) == m_ResourceTable.end())
        return true;

    AssertGraphics(
        m_RaytracingResourceDescriptionTable.find(resourceName) != m_RaytracingResourceDescriptionTable.end(),
        "If the resource never existed, there should not be any cached desc with the same resourceName");

    VisualBatch* vbOld = (VisualBatch*)m_RaytracingResourceDescriptionTable.at(resourceName).m_VisualBatch;
    VisualBatch* vbNew = (VisualBatch*)desc.m_VisualBatch;

    if (*vbOld != *vbNew)
        return true;

    return false;
}

bool Ether::Graphics::ResourceContext::ShouldRecreateResource(
    const char* resourceName,
    const RhiRaytracingShaderBindingTableDesc& desc)
{
    if (m_ResourceTable.find(resourceName) == m_ResourceTable.end())
        return true;

    AssertGraphics(
        m_RaytracingShaderBindingsTable.find(resourceName) != m_RaytracingShaderBindingsTable.end(),
        "If the resource never existed, there should not be any cached desc with the same resourceName");

    if (m_RaytracingShaderBindingsTable.at(resourceName) != desc)
        return true;

    return false;
}

bool Ether::Graphics::ResourceContext::ShouldRecreateView(const char* viewName)
{
    return m_DescriptorTable.find(viewName) == m_DescriptorTable.end();
}

void Ether::Graphics::ResourceContext::InvalidateViews(const char* resourceName)
{
    if (m_ResourceTable.find(resourceName) == m_ResourceTable.end())
        return;

    const uint64_t resourceID = m_ResourceTable.at(resourceName)->GetResourceID();
    for (auto iter = m_DescriptorTable.begin(); iter != m_DescriptorTable.end();)
    {
        if (iter->second->GetResourceID() == resourceID)
        {
            auto allocIter = m_DescriptorAllocations.find(iter->first);
            if (allocIter != m_DescriptorAllocations.end())
                m_DescriptorAllocations.erase(allocIter);

            iter = m_DescriptorTable.erase(iter);
        }
        else
            ++iter;
    }
}

void Ether::Graphics::ResourceContext::RecompilePipelineStates()
{
    for (auto& psoPair : m_CachedPipelineStates)
        RegisterPipelineState("Recompiled Pipeline State (Shader Hot Reload Only)", *psoPair.first);
}

