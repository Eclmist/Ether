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

void Ether::Graphics::BindlessDescriptorManager::RegisterAsShaderResourceView(
    StringID resourceGuid,
    const RhiResource* resource,
    RhiFormat format)
{
    if (m_GuidToIndexMap.find(resourceGuid) != m_GuidToIndexMap.end())
        LogGraphicsError("Resource GUID %s has already been registered", resourceGuid.GetString());

    auto allocation = GraphicCore::GetSrvCbvUavAllocator().Allocate(1);
    uint32_t indexInHeap = allocation->GetOffset();

    RhiShaderResourceViewDesc srvDesc = {};
    srvDesc.m_Resource = const_cast<RhiResource*>(resource);
    srvDesc.m_Format = format;
    srvDesc.m_Dimensions = RhiShaderResourceDimension::Texture2D;
    srvDesc.m_TargetCpuAddress = ((DescriptorAllocation&)(*allocation)).GetCpuAddress();
    srvDesc.m_TargetGpuAddress = ((DescriptorAllocation&)(*allocation)).GetGpuAddress();

    auto srv = GraphicCore::GetDevice().CreateShaderResourceView(srvDesc);
    m_Allocations[resourceGuid] = std::move(allocation);
    m_GuidToIndexMap[resourceGuid] = indexInHeap;
}

void Ether::Graphics::BindlessDescriptorManager::RegisterAsShaderResourceView(
    StringID resourceGuid,
    uint32_t indexInHeap)
{
    m_GuidToIndexMap[resourceGuid] = indexInHeap;

}

uint32_t Ether::Graphics::BindlessDescriptorManager::GetDescriptorIndex(StringID guid) const
{
    if (m_GuidToIndexMap.find(guid) == m_GuidToIndexMap.end())
        return 0;

    return m_GuidToIndexMap.at(guid);
}
