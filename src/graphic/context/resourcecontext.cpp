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

#include "resourcecontext.h"
#include "graphic/rhi/rhidevice.h"

ETH_NAMESPACE_BEGIN

static constexpr RHIFormat DepthStencilFormat = RHIFormat::D24UnormS8Uint;

void ResourceContext::CreateTexture2DResource(uint32_t width, uint32_t height, RHIResourceHandle& resource)
{
}

void ResourceContext::CreateDepthStencilResource(uint32_t width, uint32_t height, RHIResourceHandle& resource)
{
    RHIClearValue clearValue = { DepthStencilFormat, { 1.0, 0 } };
    RHICommitedResourceDesc desc = {};
    desc.m_HeapType = RHIHeapType::Default;
    desc.m_State = RHIResourceState::DepthWrite;
    desc.m_ClearValue = &clearValue;
    desc.m_ResourceDesc = RHICreateDepthStencilResourceDesc(DepthStencilFormat, width, height);

    CreateResource(desc, resource);
}

void ResourceContext::CreateRenderTargetView(const RHIRenderTargetViewDesc& desc, RHIRenderTargetViewHandle& view)
{
    if (ResourceExists(view.GetName()))
        return;

	GraphicCore::GetDevice()->CreateRenderTargetView(desc, view);
    m_ResourceEntries.emplace(view.GetName());
}

void ResourceContext::CreateDepthStencilView(RHIResourceHandle resource, RHIDepthStencilViewHandle& view)
{
    if (ResourceExists(view.GetName()) && !ShouldRecreateView(resource.GetName()))
        return;

	RHIDepthStencilViewDesc dsvDesc = {};
	dsvDesc.m_Format = DepthStencilFormat;
	dsvDesc.m_Resource = resource;
	GraphicCore::GetDevice()->CreateDepthStencilView(dsvDesc, view);
    m_ResourceEntries.emplace(view.GetName());
}

void ResourceContext::CreateShaderResourceView(const RHIShaderResourceViewDesc& desc, RHIShaderResourceViewHandle& view)
{
    if (ResourceExists(view.GetName()))
        return;

	GraphicCore::GetDevice()->CreateShaderResourceView(desc, view);
    m_ResourceEntries.emplace(view.GetName());
}

void ResourceContext::CreateConstantBufferView(const RHIConstantBufferViewDesc& desc, RHIConstantBufferViewHandle& view)
{
    if (ResourceExists(view.GetName()))
        return;

	GraphicCore::GetDevice()->CreateConstantBufferView(desc, view);
    m_ResourceEntries.emplace(view.GetName());
}

void ResourceContext::CreateUnorderedAccessView(const RHIUnorderedAccessViewDesc& desc, RHIUnorderedAccessViewHandle& view)
{
    if (ResourceExists(view.GetName()))
        return;

	GraphicCore::GetDevice()->CreateUnorderedAccessView(desc, view);
    m_ResourceEntries.emplace(view.GetName());
}

void ResourceContext::Reset()
{
    m_NewlyCreatedResources.clear();
}

void ResourceContext::CreateResource(const RHICommitedResourceDesc& desc, RHIResourceHandle& resource)
{
    if (ResourceExists(resource.GetName()) && !ShouldRecreateResource(resource.GetName(), desc))
        return;

    if (!resource.IsNull())
        resource.Destroy();

    if (GraphicCore::GetDevice()->CreateCommittedResource(desc, resource) != RHIResult::Success)
    {
        LogGraphicsError("Failed to create commited resource %s", resource.GetName());
        return;
    }

	m_ResourceEntries.emplace(resource.GetName());
	m_ResourceTable[resource.GetName()] = desc;
    m_NewlyCreatedResources.emplace(resource.GetName());
}

bool ResourceContext::ResourceExists(const std::wstring& resourceID) const
{
    AssertGraphics(resourceID != L"", "Resource name invalid - Resource context require unique names for resource table entries");
    return m_ResourceEntries.find(resourceID) != m_ResourceEntries.end();
}

bool ResourceContext::ShouldRecreateResource(const std::wstring& resourceID, const RHICommitedResourceDesc& desc)
{
    AssertGraphics(resourceID != L"", "Resource name invalid - Resource context require unique names for resource table entries");

    if (!ResourceExists(resourceID))
        return true;

    auto oldDesc = m_ResourceTable[resourceID];

    if (oldDesc.m_ResourceDesc.m_Width != desc.m_ResourceDesc.m_Width)
        return true;

    if (oldDesc.m_ResourceDesc.m_Height != desc.m_ResourceDesc.m_Height)
        return true;

    if (oldDesc.m_ResourceDesc.m_DepthOrArraySize != desc.m_ResourceDesc.m_DepthOrArraySize)
        return true;

    return false;
}

bool ResourceContext::ShouldRecreateView(const std::wstring& resourceID)
{
    return m_NewlyCreatedResources.find(resourceID) != m_NewlyCreatedResources.end();
}

ETH_NAMESPACE_END

