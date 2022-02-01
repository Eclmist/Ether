/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "resourcecontext.h"
#include "graphic/rhi/rhidevice.h"

ETH_NAMESPACE_BEGIN

ResourceContext::ResourceContext(CommandContext& context, const std::wstring& name)
	: m_Context(&context)
{
}

bool ResourceContext::CreateBufferResource(uint32_t size, RHIResourceHandle& resource)
{
    RHICommitedResourceDesc desc = {};
    desc.m_HeapType = RHIHeapType::Default;
    desc.m_State = RHIResourceState::Common;
    desc.m_ResourceDesc = RHICreateBufferResourceDesc(size);

    return CreateResource(desc, resource);
}

bool ResourceContext::CreateTexture2DResource(uint32_t width, uint32_t height, RHIFormat format, RHIResourceHandle& resource)
{
    RHIClearValue clearValue = { format, { 0, 0, 0, 0 } };
    RHICommitedResourceDesc desc = {};
    desc.m_HeapType = RHIHeapType::Default;
    desc.m_State = RHIResourceState::Common;
    desc.m_ClearValue = &clearValue;
    desc.m_ResourceDesc = RHICreateTexture2DResourceDesc(format, width, height);

    return CreateResource(desc, resource);
}

bool ResourceContext::CreateDepthStencilResource(uint32_t width, uint32_t height, RHIFormat format, RHIResourceHandle& resource)
{
    RHIClearValue clearValue = { format, { 1.0, 0 } };
    RHICommitedResourceDesc desc = {};
    desc.m_HeapType = RHIHeapType::Default;
    desc.m_State = RHIResourceState::DepthWrite;
    desc.m_ClearValue = &clearValue;
    desc.m_ResourceDesc = RHICreateDepthStencilResourceDesc(format, width, height);

    return CreateResource(desc, resource);
}

bool ResourceContext::CreateRenderTargetView(RHIResourceHandle resource, RHIRenderTargetViewHandle& view)
{
    if (!ShouldRecreateView(resource.GetName()))
        return false;

    RHIRenderTargetViewDesc rtvDesc = {};
    rtvDesc.m_Format = GetResourceFormat(resource.GetName());
    rtvDesc.m_Resource = resource;
    GraphicCore::GetDevice()->CreateRenderTargetView(rtvDesc, view);
    m_ResourceEntries.emplace(view.GetName());

    return true;
}

bool ResourceContext::CreateDepthStencilView(RHIResourceHandle resource, RHIDepthStencilViewHandle& view)
{
    if (!ShouldRecreateView(resource.GetName()))
        return false;

	RHIDepthStencilViewDesc dsvDesc = {};
	dsvDesc.m_Format = GetResourceFormat(resource.GetName());
	dsvDesc.m_Resource = resource;
	GraphicCore::GetDevice()->CreateDepthStencilView(dsvDesc, view);
    m_ResourceEntries.emplace(view.GetName());

    return true;
}

bool ResourceContext::CreateShaderResourceView(RHIResourceHandle resource, RHIShaderResourceViewHandle& view)
{
    if (!ShouldRecreateView(resource.GetName()))
        return false;

    RHIShaderResourceViewDesc srvDesc = {};
    srvDesc.m_Format = GetResourceFormat(resource.GetName());
    srvDesc.m_Dimensions = RHIShaderResourceDims::Texture2D;
    srvDesc.m_Resource = resource;
    GraphicCore::GetDevice()->CreateShaderResourceView(srvDesc, view);
    m_ResourceEntries.emplace(view.GetName());

    return true;
}

bool ResourceContext::CreateConstantBufferView(RHIResourceHandle resource, RHIConstantBufferViewHandle& view)
{
    if (!ShouldRecreateView(resource.GetName()))
        return false;

    RHIConstantBufferViewDesc cbvDesc = {};
    cbvDesc.m_Resource = resource;
    GraphicCore::GetDevice()->CreateConstantBufferView(cbvDesc, view);
    m_ResourceEntries.emplace(view.GetName());

    return true;
}

bool ResourceContext::CreateUnorderedAccessView(RHIResourceHandle resource, RHIUnorderedAccessViewHandle& view)
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

bool ResourceContext::CreateResource(const RHICommitedResourceDesc& desc, RHIResourceHandle& resource)
{
    if (!ShouldRecreateResource(resource.GetName(), desc))
        return false;

    if (!resource.IsNull())
        resource.Destroy();

    if (GraphicCore::GetDevice()->CreateCommittedResource(desc, resource) != RHIResult::Success)
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

bool ResourceContext::ShouldRecreateResource(const std::wstring& resourceID, const RHICommitedResourceDesc& desc) const
{
    AssertGraphics(resourceID != L"", "Resource name invalid - Resource context require unique names for resource table entries");

    if (!ResourceExists(resourceID))
        return true;

    auto oldDesc = m_ResourceTable.at(resourceID);

    if (oldDesc.m_ResourceDesc.m_Width != desc.m_ResourceDesc.m_Width)
        return true;

    if (oldDesc.m_ResourceDesc.m_Height != desc.m_ResourceDesc.m_Height)
        return true;

    if (oldDesc.m_ResourceDesc.m_DepthOrArraySize != desc.m_ResourceDesc.m_DepthOrArraySize)
        return true;

    return false;
}

bool ResourceContext::ShouldRecreateView(const std::wstring& resourceID) const
{
    AssertGraphics(resourceID != L"", "Resource name invalid - Resource context require unique names for resource table entries");

    if (!ResourceExists(resourceID))
        return true;

    return m_NewlyCreatedResources.find(resourceID) != m_NewlyCreatedResources.end();
}

RHIFormat ResourceContext::GetResourceFormat(const std::wstring& resourceID) const
{
    if (!ResourceExists(resourceID))
        return RHIFormat::Unknown;

    return m_ResourceTable.find(resourceID)->second.m_ResourceDesc.m_Format;
}

ETH_NAMESPACE_END

