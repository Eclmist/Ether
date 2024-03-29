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

#include "graphics/memory/bindlessdescriptormanager.h"
#include "graphics/graphiccore.h"
#include "graphics/rhi/rhiresourceviews.h"
#include "graphics/common/vertexformats.h"

uint32_t Ether::Graphics::BindlessDescriptorManager::RegisterAsShaderResourceView(
    StringID resourceGuid,
    const RhiResource& resource,
    RhiFormat format)
{
    if (m_GuidToIndexMap.find(resourceGuid) != m_GuidToIndexMap.end())
        LogGraphicsError("Resource GUID %s has already been registered", resourceGuid.GetString().c_str());

    auto allocation = GraphicCore::GetSrvCbvUavAllocator().Allocate(1);
    uint32_t indexInHeap = allocation->GetOffset();

    RhiShaderResourceView srv;
    srv.SetResourceID(resource.GetResourceID());
    srv.SetFormat(format);
    srv.SetDimension(RhiResourceDimension::Texture2D);
    srv.SetCpuAddress(((DescriptorAllocation&)(*allocation)).GetCpuAddress());
    srv.SetGpuAddress(((DescriptorAllocation&)(*allocation)).GetGpuAddress());

    GraphicCore::GetDevice().InitializeShaderResourceView(srv, resource);
    m_Allocations[resourceGuid] = std::move(allocation);
    m_GuidToIndexMap[resourceGuid] = indexInHeap;
    return indexInHeap;
}

uint32_t Ether::Graphics::BindlessDescriptorManager::RegisterAsShaderResourceView(
    StringID resourceGuid,
    const RhiResource& resource,
    RhiVertexBufferViewDesc vb)
{
    if (m_GuidToIndexMap.find(resourceGuid) != m_GuidToIndexMap.end())
        LogGraphicsError("Resource GUID %s has already been registered", resourceGuid.GetString().c_str());

    auto allocation = GraphicCore::GetSrvCbvUavAllocator().Allocate(1);
    uint32_t indexInHeap = allocation->GetOffset();

    RhiShaderResourceView srv;
    srv.SetResourceID(resource.GetResourceID());
    srv.SetFormat(RhiFormat::Unknown);
    srv.SetDimension(RhiResourceDimension::StructuredBuffer);
    srv.SetCpuAddress(((DescriptorAllocation&)(*allocation)).GetCpuAddress());
    srv.SetGpuAddress(((DescriptorAllocation&)(*allocation)).GetGpuAddress());
    srv.SetWidth(vb.m_BufferSize);
    srv.SetStructuredBufferStride(vb.m_Stride);

    GraphicCore::GetDevice().InitializeShaderResourceView(srv, resource);
    m_Allocations[resourceGuid] = std::move(allocation);
    m_GuidToIndexMap[resourceGuid] = indexInHeap;
    return indexInHeap;
}

uint32_t Ether::Graphics::BindlessDescriptorManager::RegisterAsShaderResourceView(
    StringID resourceGuid,
    const RhiResource& resource,
    RhiIndexBufferViewDesc ib)
{
    if (m_GuidToIndexMap.find(resourceGuid) != m_GuidToIndexMap.end())
        LogGraphicsError("Resource GUID %s has already been registered", resourceGuid.GetString().c_str());

    auto allocation = GraphicCore::GetSrvCbvUavAllocator().Allocate(1);
    uint32_t indexInHeap = allocation->GetOffset();

    RhiShaderResourceView srv;
    srv.SetResourceID(resource.GetResourceID());
    srv.SetDimension(RhiResourceDimension::StructuredBuffer);
    srv.SetCpuAddress(((DescriptorAllocation&)(*allocation)).GetCpuAddress());
    srv.SetGpuAddress(((DescriptorAllocation&)(*allocation)).GetGpuAddress());
    srv.SetWidth(ib.m_BufferSize);
    srv.SetStructuredBufferStride(sizeof(uint32_t));

    GraphicCore::GetDevice().InitializeShaderResourceView(srv, resource);
    m_Allocations[resourceGuid] = std::move(allocation);
    m_GuidToIndexMap[resourceGuid] = indexInHeap;
    return indexInHeap;
}

uint32_t Ether::Graphics::BindlessDescriptorManager::GetDescriptorIndex(StringID guid) const
{
    if (m_GuidToIndexMap.find(guid) == m_GuidToIndexMap.end())
        return 0;

    return m_GuidToIndexMap.at(guid);
}

uint32_t Ether::Graphics::BindlessDescriptorManager::RegisterSampler(StringID name, RhiSamplerParameterDesc& sampler)
{
    if (m_GuidToIndexMap.find(name) != m_GuidToIndexMap.end())
        LogGraphicsError("Sampler %s has already been registered", name.GetString().c_str());

    auto allocation = GraphicCore::GetSamplerAllocator().Allocate(1);
    uint32_t indexInHeap = allocation->GetOffset();

    GraphicCore::GetDevice().CopySampler(sampler, ((DescriptorAllocation&)(*allocation)).GetCpuAddress());
    m_Allocations[name] = std::move(allocation);
    m_GuidToIndexMap[name] = indexInHeap;

    return indexInHeap;
}

