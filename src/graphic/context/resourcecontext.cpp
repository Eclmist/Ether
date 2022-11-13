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

#include "resourcecontext.h"
#include "graphic/rhi/rhidevice.h"

ETH_NAMESPACE_BEGIN

ResourceContext::ResourceContext(CommandContext& context, const std::wstring& name)
    : m_Context(&context)
    , m_RtvDescriptorAllocator(RhiDescriptorHeapType::Rtv, false, 4096)
    , m_DsvDescriptorAllocator(RhiDescriptorHeapType::Dsv, false, 4096)
{
}

bool ResourceContext::CreateConstantBuffer(uint32_t size, RhiResourceHandle& resource)
{
    RhiCommitedResourceDesc desc = {};
    desc.m_HeapType = RhiHeapType::Upload;
    desc.m_State = RhiResourceState::GenericRead;
    desc.m_ResourceDesc = RhiCreateBufferResourceDesc(size);

    return CreateResource(desc, resource);
}

bool ResourceContext::CreateTexture2DResource(uint32_t width, uint32_t height, RhiFormat format, RhiResourceHandle& resource)
{
    RhiClearValue clearValue = { format, { 0, 0, 0, 0 } };
    RhiCommitedResourceDesc desc = {};
    desc.m_HeapType = RhiHeapType::Default;
    desc.m_State = RhiResourceState::Common;
    desc.m_ClearValue = &clearValue;
    desc.m_ResourceDesc = RhiCreateTexture2DResourceDesc(format, width, height);

    return CreateResource(desc, resource);
}

bool ResourceContext::CreateDepthStencilResource(uint32_t width, uint32_t height, RhiFormat format, RhiResourceHandle& resource)
{
    RhiClearValue clearValue = { format, { 1.0, 0 } };
    RhiCommitedResourceDesc desc = {};
    desc.m_HeapType = RhiHeapType::Default;
    desc.m_State = RhiResourceState::DepthWrite;
    desc.m_ClearValue = &clearValue;
    desc.m_ResourceDesc = RhiCreateDepthStencilResourceDesc(format, width, height);

    return CreateResource(desc, resource);
}

bool ResourceContext::CreateRenderTargetView(RhiResourceHandle resource, RhiRenderTargetViewHandle& view)
{
    if (!ShouldRecreateView(resource.GetName(), view.GetName()))
        return false;

    DescriptorAllocation alloc = m_RtvDescriptorAllocator.Allocate();
    m_DescriptorTable.emplace(view.GetName(), std::move(alloc));

    RhiRenderTargetViewDesc rtvDesc = {};
    rtvDesc.m_Format = GetResourceFormat(resource.GetName());
    rtvDesc.m_Resource = resource;
    rtvDesc.m_TargetCpuHandle = alloc.GetCpuHandle();
    GraphicCore::GetDevice()->CreateRenderTargetView(rtvDesc, view);
    m_ResourceEntries.emplace(view.GetName());
    m_NewlyCreatedResources.emplace(view.GetName());

    return true;
}

bool ResourceContext::CreateDepthStencilView(RhiResourceHandle resource, RhiDepthStencilViewHandle& view)
{
    if (!ShouldRecreateView(resource.GetName(), view.GetName()))
        return false;

    DescriptorAllocation alloc = m_DsvDescriptorAllocator.Allocate();
    m_DescriptorTable.emplace(view.GetName(), std::move(alloc));

    RhiDepthStencilViewDesc dsvDesc = {};
    dsvDesc.m_Format = GetResourceFormat(resource.GetName());
    dsvDesc.m_Resource = resource;
    dsvDesc.m_TargetCpuHandle = alloc.GetCpuHandle();
    GraphicCore::GetDevice()->CreateDepthStencilView(dsvDesc, view);
    m_ResourceEntries.emplace(view.GetName());
    m_NewlyCreatedResources.emplace(view.GetName());

    return true;
}

bool ResourceContext::CreateShaderResourceView(RhiResourceHandle resource, RhiShaderResourceViewHandle& view)
{
    if (!ShouldRecreateView(resource.GetName(), view.GetName()))
        return false;

    DescriptorAllocation alloc = GraphicCore::GetGpuDescriptorAllocator().Allocate();
    m_DescriptorTable.emplace(view.GetName(), std::move(alloc));

    RhiShaderResourceViewDesc srvDesc = {};
    srvDesc.m_Format = GetResourceFormat(resource.GetName());
    srvDesc.m_Dimensions = RhiShaderResourceDims::Texture2D;
    srvDesc.m_Resource = resource;
    srvDesc.m_TargetCpuHandle = alloc.GetCpuHandle();
    srvDesc.m_TargetGpuHandle = alloc.GetGpuHandle();
    GraphicCore::GetDevice()->CreateShaderResourceView(srvDesc, view);
    GraphicCore::GetBindlessResourceManager().RegisterView<RhiShaderResourceView>(view, alloc.GetDescriptorIndex());
    m_ResourceEntries.emplace(view.GetName());
    m_NewlyCreatedResources.emplace(view.GetName());

    return true;
}

bool ResourceContext::CreateConstantBufferView(uint32_t bufferSize, RhiResourceHandle resource, RhiConstantBufferViewHandle& view)
{
    if (!ShouldRecreateView(resource.GetName(), view.GetName()))
        return false;

    DescriptorAllocation alloc = GraphicCore::GetGpuDescriptorAllocator().Allocate();
    m_DescriptorTable.emplace(view.GetName(), std::move(alloc));

    RhiConstantBufferViewDesc cbvDesc = {};
    cbvDesc.m_Resource = resource;
    cbvDesc.m_BufferSize = bufferSize;
    cbvDesc.m_TargetCpuHandle = alloc.GetCpuHandle();
    cbvDesc.m_TargetGpuHandle = alloc.GetGpuHandle();
    GraphicCore::GetDevice()->CreateConstantBufferView(cbvDesc, view);
    GraphicCore::GetBindlessResourceManager().RegisterView<RhiConstantBufferView>(view, alloc.GetDescriptorIndex());
    m_ResourceEntries.emplace(view.GetName());
    m_NewlyCreatedResources.emplace(view.GetName());

    return true;
}

bool ResourceContext::CreateUnorderedAccessView(RhiResourceHandle resource, RhiUnorderedAccessViewHandle& view)
{
    if (ResourceExists(view.GetName()))
        return false;

    return false;
}

bool ResourceContext::InitializeTexture2D(CompiledTexture& texture)
{
    bool resourceRecreated = CreateTexture2DResource(texture.GetWidth(), texture.GetHeight(), texture.GetFormat(), texture.GetResource());
    bool viewRecreated = CreateShaderResourceView(texture.GetResource(), texture.GetView());

    // TODO: multiple of the same view can be created in a frame where the resource was created. This is a waste.
    //AssertGraphics(resourceRecreated == viewRecreated, "Resource and view should either both be recreated or both not");

    if (!resourceRecreated)
        return false;

    m_Context->InitializeTexture(texture);
    return true;
}

void ResourceContext::Reset()
{
    m_NewlyCreatedResources.clear();
}

bool ResourceContext::CreateResource(const RhiCommitedResourceDesc& desc, RhiResourceHandle& resource)
{
    if (!ShouldRecreateResource(resource.GetName(), desc))
        return false;

    if (!resource.IsNull())
        resource.Destroy();

    if (GraphicCore::GetDevice()->CreateCommittedResource(desc, resource) != RhiResult::Success)
    {
        LogGraphicsError("Failed to create commited resource %s", resource.GetName());
        return false;
    }

    m_ResourceEntries.emplace(resource.GetName());
    m_ResourceTable[resource.GetName()] = desc;
    m_NewlyCreatedResources.emplace(resource.GetName());

    return true;
}

bool ResourceContext::ResourceExists(const std::wstring& resourceID) const
{
    AssertGraphics(resourceID != L"", "Resource name invalid - Resource context require unique names for resource table entries");
    return m_ResourceEntries.find(resourceID) != m_ResourceEntries.end();
}

bool ResourceContext::ShouldRecreateResource(const std::wstring& resourceID, const RhiCommitedResourceDesc& desc) const
{
    AssertGraphics(resourceID != L"", "Resource name invalid - Resource context require unique names for resource table entries");

    if (!ResourceExists(resourceID))
        return true;

    RhiCommitedResourceDesc oldDesc = m_ResourceTable.at(resourceID);

    if (oldDesc.m_ResourceDesc.m_Width != desc.m_ResourceDesc.m_Width)
        return true;

    if (oldDesc.m_ResourceDesc.m_Height != desc.m_ResourceDesc.m_Height)
        return true;

    if (oldDesc.m_ResourceDesc.m_DepthOrArraySize != desc.m_ResourceDesc.m_DepthOrArraySize)
        return true;

    return false;
}

bool ResourceContext::ShouldRecreateView(const std::wstring& resourceName, std::wstring& viewName) const
{
    AssertGraphics(resourceName != L"", "Resource name invalid - Resource context require unique names for resource table entries");

    if (!ResourceExists(resourceName))
        return true;

    return m_NewlyCreatedResources.find(resourceName) != m_NewlyCreatedResources.end() &&
           m_NewlyCreatedResources.find(viewName) == m_NewlyCreatedResources.end();
}

RhiFormat ResourceContext::GetResourceFormat(const std::wstring& resourceID) const
{
    if (!ResourceExists(resourceID))
        return RhiFormat::Unknown;

    return m_ResourceTable.find(resourceID)->second.m_ResourceDesc.m_Format;
}

ETH_NAMESPACE_END

