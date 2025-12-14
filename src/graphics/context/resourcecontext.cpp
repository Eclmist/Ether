/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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
    Reset();
}

void Ether::Graphics::ResourceContext::Reset()
{
    m_StagingSrvCbvUavAllocator = std::make_unique<DescriptorAllocator>(
        RhiDescriptorHeapType::SrvCbvUav,
        _64KiB,
        false);

    m_ResourceDescriptionTable.clear();
    m_RaytracingShaderBindingsTable.clear();
    m_RaytracingResourceDescriptionTable.clear();
    m_DescriptorTable.clear();
    m_ResourceTable.clear();
    m_DescriptorAllocations.clear();
}

void Ether::Graphics::ResourceContext::RegisterPipelineState(const char* name, RhiPipelineStateDesc& pipelineStateDesc)
{
    // This caching doesn't actually work since it just cheats by using the address as a key
    // A mechanism for hashing the pipeline state data. This means that if two different pipeline states
    // were registered, both will be compiled even if they are identical.
    // TODO: maybe inherit serializable and create a stringstream?
    //
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

    if (!m_CachedPipelineStates.contains(&pipelineStateDesc))
        m_CachedPipelineStates[&pipelineStateDesc] = pipelineStateDesc.Compile(name);
}

Ether::Graphics::RhiPipelineState& Ether::Graphics::ResourceContext::GetPipelineState(
    RhiPipelineStateDesc& pipelineStateDesc)
{
    if (!m_CachedPipelineStates.contains(&pipelineStateDesc))
    {
        LogGraphicsError("A pipeline state desc was used before registration");
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
    InvalidateResource(resourceName);
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
        desc.m_State = RhiResourceState::DepthWrite;

        // RTCamp11 Hack
        // Force this specific format for DS resource
        desc.m_ResourceDesc.m_Format = DepthBufferFormat;
        desc.m_ClearValue = { DepthBufferDsvFormat, { 0, 0 } };
    }

    if (!ShouldRecreateResource(resourceName, desc))
        return *m_ResourceTable.at(resourceName);

    InvalidateViews(resourceName);
    InvalidateResource(resourceName);
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
    InvalidateResource(resourceName);
    m_ResourceTable[resourceName] = GraphicCore::GetDevice().CreateCommittedResource(desc);
    m_ResourceDescriptionTable[resourceName] = desc;

    return *m_ResourceTable.at(resourceName);
}

Ether::Graphics::RhiResource& Ether::Graphics::ResourceContext::CreateAccelerationStructure(
    const char* resourceName,
    const RhiTopLevelAccelerationStructureDesc& desc, 
    GraphicContext& gfxContext)
{
    if (ShouldRecreateResource(resourceName, desc))
    {
        const bool resourceExists = m_TopLevelAccelerationStructure != nullptr;
        if (resourceExists)
        {
            m_StaleAccelerationStructures.emplace(m_TopLevelAccelerationStructure);
            if (m_StaleAccelerationStructures.size() > 4)
                m_StaleAccelerationStructures.pop();
        }

        InvalidateViews(resourceName);
        InvalidateResource(resourceName);
        m_TopLevelAccelerationStructure = GraphicCore::GetDevice().CreateAccelerationStructure(desc);
        m_ResourceTable[resourceName] = m_TopLevelAccelerationStructure->m_DataBuffer;
        m_RaytracingResourceDescriptionTable[resourceName] = desc;

        gfxContext.TransitionResource(*m_TopLevelAccelerationStructure->m_ScratchBuffer, RhiResourceState::UnorderedAccess);
        gfxContext.BuildAccelerationStructure(*m_TopLevelAccelerationStructure);
        gfxContext.FinalizeAndExecute(!resourceExists);
        gfxContext.Reset();
    }

    return *m_ResourceTable.at(resourceName);
}

Ether::Graphics::RhiResource& Ether::Graphics::ResourceContext::CreateRaytracingShaderBindingTable(
    const char* resourceName,
    const RhiRaytracingShaderBindingTableDesc& desc)
{
    if (!ShouldRecreateResource(resourceName, desc))
        return *m_ResourceTable.at(resourceName);

    InvalidateViews(resourceName);
    InvalidateResource(resourceName);

    m_ResourceTable[resourceName] = GraphicCore::GetDevice().CreateRaytracingShaderBindingTable(resourceName, desc);
    m_RaytracingShaderBindingsTable[resourceName] = desc;

    return *m_ResourceTable.at(resourceName);
}

void Ether::Graphics::ResourceContext::InitializeRenderTargetView(RhiResourceView* view)
{
    if (!ShouldRecreateView(view->GetViewID()))
        return;

    auto alloc = GraphicCore::GetRtvAllocator().Allocate();
    view->SetCpuAddress(((DescriptorAllocation&)(*alloc)).GetCpuAddress());

    GraphicCore::GetDevice().InitializeRenderTargetView((*(RhiRenderTargetView*)(view)), *m_ResourceTable[view->GetResourceID()]);
    m_DescriptorTable[view->GetViewID()] = view;
    m_DescriptorAllocations[view->GetViewID()] = std::move(alloc);
}

void Ether::Graphics::ResourceContext::InitializeDepthStencilView(RhiResourceView* view)
{
    if (!ShouldRecreateView(view->GetViewID()))
        return;

    auto alloc = GraphicCore::GetDsvAllocator().Allocate();
    view->SetCpuAddress(((DescriptorAllocation&)(*alloc)).GetCpuAddress());

    GraphicCore::GetDevice().InitializeDepthStencilView((*(RhiDepthStencilView*)(view)), *m_ResourceTable[view->GetResourceID()]);
    m_DescriptorTable[view->GetViewID()] = view;
    m_DescriptorAllocations[view->GetViewID()] = std::move(alloc);
}

void Ether::Graphics::ResourceContext::InitializeShaderResourceView(RhiResourceView* view)
{
    if (!ShouldRecreateView(view->GetViewID()))
        return;

    auto alloc = GraphicCore::GetSrvCbvUavAllocator().Allocate();
    view->SetCpuAddress(((DescriptorAllocation&)(*alloc)).GetCpuAddress());
    ((RhiShaderResourceView*)view)->SetGpuAddress(((DescriptorAllocation&)(*alloc)).GetGpuAddress());

    GraphicCore::GetDevice().InitializeShaderResourceView((*(RhiShaderResourceView*)(view)), *m_ResourceTable[view->GetResourceID()]);
    m_DescriptorTable[view->GetViewID()] = view;
    m_DescriptorAllocations[view->GetViewID()] = std::move(alloc);
}

void Ether::Graphics::ResourceContext::InitializeUnorderedAccessView(RhiResourceView* view)
{
    if (!ShouldRecreateView(view->GetViewID()))
        return;

    auto alloc = GraphicCore::GetSrvCbvUavAllocator().Allocate();
    view->SetCpuAddress(((DescriptorAllocation&)(*alloc)).GetCpuAddress());
    ((RhiUnorderedAccessView*)view)->SetGpuAddress(((DescriptorAllocation&)(*alloc)).GetGpuAddress());

    GraphicCore::GetDevice().InitializeUnorderedAccessView((*(RhiUnorderedAccessView*)(view)), *m_ResourceTable[view->GetResourceID()]);
    m_DescriptorTable[view->GetViewID()] = view;
    m_DescriptorAllocations[view->GetViewID()] = std::move(alloc);
}

void Ether::Graphics::ResourceContext::InitializeConstantBufferView(RhiResourceView* view)
{
    if (!ShouldRecreateView(view->GetViewID()))
        return;

    auto alloc = GraphicCore::GetSrvCbvUavAllocator().Allocate();
    view->SetCpuAddress(((DescriptorAllocation&)(*alloc)).GetCpuAddress());
    ((RhiConstantBufferView*)view)->SetGpuAddress(((DescriptorAllocation&)(*alloc)).GetGpuAddress());

    GraphicCore::GetDevice().InitializeConstantBufferView((*(RhiConstantBufferView*)(view)), *m_ResourceTable[view->GetResourceID()]);
    m_DescriptorTable[view->GetViewID()] = view;
    m_DescriptorAllocations[view->GetViewID()] = std::move(alloc);
}

bool Ether::Graphics::ResourceContext::ShouldRecreateResource(StringID resourceID, const RhiCommitedResourceDesc& desc)
{
    // If the resource don't exist in the resource table at all
    if (!m_ResourceTable.contains(resourceID))
        return true;

    AssertGraphics(
        m_ResourceDescriptionTable.contains(resourceID),
        "If the resource never existed, there should not be any cached desc with the same resourceID");

    // If the resource exist, but it's description has changed
    if (std::memcmp(
            &m_ResourceDescriptionTable.at(resourceID).m_ResourceDesc,
            &desc.m_ResourceDesc,
            sizeof(RhiResourceDesc)) != 0)
        return true;

    return false;
}

Ether::Graphics::RhiResource* Ether::Graphics::ResourceContext::GetResource(const RhiResourceView* view) const
{
    if (!m_ResourceTable.contains(view->GetResourceID()))
        LogGraphicsFatal("The requested resource (%s) has not yet been created", view->GetResourceID().GetString().c_str());

    return m_ResourceTable.at(view->GetResourceID()).get();
}

bool Ether::Graphics::ResourceContext::ShouldRecreateResource(
    StringID resourceID,
    const RhiTopLevelAccelerationStructureDesc& desc)
{
    // If the resource don't exist in the resource table at all
    if (!m_ResourceTable.contains(resourceID))
        return true;

    AssertGraphics(
        m_RaytracingResourceDescriptionTable.contains(resourceID),
        "If the resource never existed, there should not be any cached desc with the same resourceID");

    if (m_RaytracingResourceDescriptionTable.at(resourceID).m_NumVisuals != desc.m_NumVisuals)
        return true;

    for (uint32_t i = 0; i < desc.m_NumVisuals; ++i)
    {
        Visual vbOld = ((Visual*)m_RaytracingResourceDescriptionTable.at(resourceID).m_Visuals)[i];
        Visual vbNew = ((Visual*)desc.m_Visuals)[i];

        if (vbOld.m_ModelMatrix != vbNew.m_ModelMatrix)
            return true;

        if (vbOld.m_Mesh != vbNew.m_Mesh)
            return true;
    }

    return false;
}

bool Ether::Graphics::ResourceContext::ShouldRecreateResource(
    StringID resourceID,
    const RhiRaytracingShaderBindingTableDesc& desc)
{
    if (!m_ResourceTable.contains(resourceID))
        return true;

    AssertGraphics(
        m_RaytracingShaderBindingsTable.contains(resourceID),
        "If the resource never existed, there should not be any cached desc with the same resourceID");

    if (m_RaytracingShaderBindingsTable.at(resourceID) != desc)
        return true;

    return false;
}

bool Ether::Graphics::ResourceContext::ShouldRecreateView(StringID viewID)
{
    return !m_DescriptorTable.contains(viewID);
}

void Ether::Graphics::ResourceContext::InvalidateViews(StringID resourceID)
{
    if (!m_ResourceTable.contains(resourceID))
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

void Ether::Graphics::ResourceContext::InvalidateResource(StringID resourceID)
{
    if (!m_ResourceTable.contains(resourceID))
        return;

    m_StaleResources.push(m_ResourceTable.at(resourceID));
    m_ResourceTable.erase(resourceID);
    
    // Start deallocating stale resources once there's too many (64 is arbitrary)
    // Should really track resource lifetime (TODO)
    if (m_StaleResources.size() > 64)
        m_StaleResources.pop();
}

void Ether::Graphics::ResourceContext::ReloadPipelineStates()
{
    for (auto& psoPair : m_CachedPipelineStates)
        RegisterPipelineState("Recompiled Pipeline State (Shader Hot Reload Only)", *psoPair.first);
}
