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
    m_StagingSrvCbvUavAllocator = std::make_unique<DescriptorAllocator>(
        RhiDescriptorHeapType::SrvCbvUav,
        _64KiB,
        false);
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

Ether::Graphics::RhiResource& Ether::Graphics::ResourceContext::CreateBufferResource(
    const char* resourceName,
    size_t size,
    RhiResourceFlag flags)
{
    RhiCommitedResourceDesc desc = {};
    desc.m_Name = resourceName;
    desc.m_HeapType = RhiHeapType::Default;
    desc.m_State = RhiResourceState::Common;
    desc.m_ResourceDesc = RhiCreateBufferResourceDesc(size);
    desc.m_ResourceDesc.m_Flag = flags;

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
    RhiFormat format,
    RhiResourceFlag flags)
{
    RhiCommitedResourceDesc desc = {};
    desc.m_Name = resourceName;
    desc.m_HeapType = RhiHeapType::Default;
    desc.m_State = RhiResourceState::Common;
    desc.m_ClearValue = { format, { 0, 0, 0, 0 } };
    desc.m_ResourceDesc = RhiCreateTexture2DResourceDesc(format, resolution);
    desc.m_ResourceDesc.m_Flag = flags;

    if ((flags & RhiResourceFlag::AllowDepthStencil) != RhiResourceFlag::None)
    {
        desc.m_ClearValue = { format, { 1, 0 } };
        desc.m_State = RhiResourceState::DepthWrite;
    }

    if (!ShouldRecreateResource(resourceName, desc))
        return *m_ResourceTable.at(resourceName);

    InvalidateViews(resourceName);
    m_ResourceTable[resourceName] = GraphicCore::GetDevice().CreateCommittedResource(desc);
    m_ResourceDescriptionTable[resourceName] = desc;

    return *m_ResourceTable.at(resourceName);
}

Ether::Graphics::RhiResource& Ether::Graphics::ResourceContext::CreateTexture3DResource(
    const char* resourceName,
    const ethVector3u resolution,
    RhiFormat format,
    RhiResourceFlag flags)
{
    RhiCommitedResourceDesc desc = {};
    desc.m_Name = resourceName;
    desc.m_HeapType = RhiHeapType::Default;
    desc.m_State = RhiResourceState::Common;
    desc.m_ClearValue = { format, { 0, 0, 0, 0 } };
    desc.m_ResourceDesc = RhiCreateTexture3DResourceDesc(format, resolution);
    desc.m_ResourceDesc.m_Flag = flags;

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

void Ether::Graphics::ResourceContext::InitializeRenderTargetView(std::shared_ptr<RhiResourceView> view)
{
    if (!ShouldRecreateView(view->GetViewID()))
        return;

    auto alloc = GraphicCore::GetRtvAllocator().Allocate();
    view->SetCpuAddress(((DescriptorAllocation&)(*alloc)).GetCpuAddress());

    GraphicCore::GetDevice().InitializeRenderTargetView((*(RhiRenderTargetView*)(view.get())), *m_ResourceTable[view->GetResourceID()]);
    m_DescriptorTable[view->GetViewID()] = view;
    m_DescriptorAllocations[view->GetViewID()] = std::move(alloc);
}

void Ether::Graphics::ResourceContext::InitializeDepthStencilView(std::shared_ptr<RhiResourceView> view)
{
    if (!ShouldRecreateView(view->GetViewID()))
        return;

    auto alloc = GraphicCore::GetDsvAllocator().Allocate();
    view->SetCpuAddress(((DescriptorAllocation&)(*alloc)).GetCpuAddress());

    GraphicCore::GetDevice().InitializeDepthStencilView((*(RhiDepthStencilView*)(view.get())), *m_ResourceTable[view->GetResourceID()]);
    m_DescriptorTable[view->GetViewID()] = view;
    m_DescriptorAllocations[view->GetViewID()] = std::move(alloc);
}

void Ether::Graphics::ResourceContext::InitializeShaderResourceView(std::shared_ptr<RhiResourceView> view)
{
    if (!ShouldRecreateView(view->GetViewID()))
        return;

    auto alloc = GraphicCore::GetSrvCbvUavAllocator().Allocate();
    view->SetCpuAddress(((DescriptorAllocation&)(*alloc)).GetCpuAddress());
    ((RhiShaderResourceView*)view.get())->SetGpuAddress(((DescriptorAllocation&)(*alloc)).GetGpuAddress());

    GraphicCore::GetDevice().InitializeShaderResourceView((*(RhiShaderResourceView*)(view.get())), *m_ResourceTable[view->GetResourceID()]);
    m_DescriptorTable[view->GetViewID()] = view;
    m_DescriptorAllocations[view->GetViewID()] = std::move(alloc);
}

void Ether::Graphics::ResourceContext::InitializeUnorderedAccessView(std::shared_ptr<RhiResourceView> view)
{
    if (!ShouldRecreateView(view->GetViewID()))
        return;

    auto alloc = GraphicCore::GetSrvCbvUavAllocator().Allocate();
    view->SetCpuAddress(((DescriptorAllocation&)(*alloc)).GetCpuAddress());
    ((RhiUnorderedAccessView*)view.get())->SetGpuAddress(((DescriptorAllocation&)(*alloc)).GetGpuAddress());

    GraphicCore::GetDevice().InitializeUnorderedAccessView((*(RhiUnorderedAccessView*)(view.get())), *m_ResourceTable[view->GetResourceID()]);
    m_DescriptorTable[view->GetViewID()] = view;
    m_DescriptorAllocations[view->GetViewID()] = std::move(alloc);
}

void Ether::Graphics::ResourceContext::InitializeConstantBufferView(std::shared_ptr<RhiResourceView> view)
{
    if (!ShouldRecreateView(view->GetViewID()))
        return;

    auto alloc = GraphicCore::GetSrvCbvUavAllocator().Allocate();
    view->SetCpuAddress(((DescriptorAllocation&)(*alloc)).GetCpuAddress());
    ((RhiConstantBufferView*)view.get())->SetGpuAddress(((DescriptorAllocation&)(*alloc)).GetGpuAddress());

    GraphicCore::GetDevice().InitializeConstantBufferView((*(RhiConstantBufferView*)(view.get())), *m_ResourceTable[view->GetResourceID()]);
    m_DescriptorTable[view->GetViewID()] = view;
    m_DescriptorAllocations[view->GetViewID()] = std::move(alloc);
}

bool Ether::Graphics::ResourceContext::ShouldRecreateResource(StringID resourceID, const RhiCommitedResourceDesc& desc)
{
    // If the resource don't exist in the resource table at all
    if (m_ResourceTable.find(resourceID) == m_ResourceTable.end())
        return true;

    AssertGraphics(
        m_ResourceDescriptionTable.find(resourceID) != m_ResourceDescriptionTable.end(),
        "If the resource never existed, there should not be any cached desc with the same resourceID");

    // If the resource exist, but it's description has changed
    if (std::memcmp(
            &m_ResourceDescriptionTable.at(resourceID).m_ResourceDesc,
            &desc.m_ResourceDesc,
            sizeof(RhiResourceDesc)) != 0)
        return true;

    return false;
}

bool Ether::Graphics::ResourceContext::ShouldRecreateResource(
    StringID resourceID,
    const RhiTopLevelAccelerationStructureDesc& desc)
{
    // If the resource don't exist in the resource table at all
    if (m_ResourceTable.find(resourceID) == m_ResourceTable.end())
        return true;

    AssertGraphics(
        m_RaytracingResourceDescriptionTable.find(resourceID) != m_RaytracingResourceDescriptionTable.end(),
        "If the resource never existed, there should not be any cached desc with the same resourceID");

    VisualBatch* vbOld = (VisualBatch*)m_RaytracingResourceDescriptionTable.at(resourceID).m_VisualBatch;
    VisualBatch* vbNew = (VisualBatch*)desc.m_VisualBatch;

    if (*vbOld != *vbNew)
        return true;

    return false;
}

bool Ether::Graphics::ResourceContext::ShouldRecreateResource(
    StringID resourceID,
    const RhiRaytracingShaderBindingTableDesc& desc)
{
    if (m_ResourceTable.find(resourceID) == m_ResourceTable.end())
        return true;

    AssertGraphics(
        m_RaytracingShaderBindingsTable.find(resourceID) != m_RaytracingShaderBindingsTable.end(),
        "If the resource never existed, there should not be any cached desc with the same resourceID");

    if (m_RaytracingShaderBindingsTable.at(resourceID) != desc)
        return true;

    return false;
}

bool Ether::Graphics::ResourceContext::ShouldRecreateView(StringID viewID)
{
    return m_DescriptorTable.find(viewID) == m_DescriptorTable.end();
}

void Ether::Graphics::ResourceContext::InvalidateViews(StringID resourceID)
{
    if (m_ResourceTable.find(resourceID) == m_ResourceTable.end())
        return;

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
