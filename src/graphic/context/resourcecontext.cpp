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

void ResourceContext::CreateResource(const RHICommitedResourceDesc& desc, RHIResourceHandle& resource)
{
    AssertGraphics(resource.GetName() != L"", "Resource name invalid - Resource context require unique names for resource table");

    if (m_ResourceTable.find(resource.GetName()) == m_ResourceTable.end())
    {
		GraphicCore::GetDevice()->CreateCommittedResource(desc, resource);
        m_ResourceTable.emplace(resource.GetName());
    }
}

void ResourceContext::CreateRenderTargetView(const RHIRenderTargetViewDesc& desc, RHIRenderTargetViewHandle& view)
{
    AssertGraphics(view.GetName() != L"", "View name invalid - Resource context require unique names for view table");
    if (m_ResourceTable.find(view.GetName()) == m_ResourceTable.end())
    {
		GraphicCore::GetDevice()->CreateRenderTargetView(desc, view);
        m_ResourceTable.emplace(view.GetName());
    }
}

void ResourceContext::CreateDepthStencilView(const RHIDepthStencilViewDesc& desc, RHIDepthStencilViewHandle& view)
{
    AssertGraphics(view.GetName() != L"", "View name invalid - Resource context require unique names for view table");
    if (m_ResourceTable.find(view.GetName()) == m_ResourceTable.end())
    {
		GraphicCore::GetDevice()->CreateDepthStencilView(desc, view);
        m_ResourceTable.emplace(view.GetName());
    }
}

void ResourceContext::CreateShaderResourceView(const RHIShaderResourceViewDesc& desc, RHIShaderResourceViewHandle& view)
{
    AssertGraphics(view.GetName() != L"", "View name invalid - Resource context require unique names for view table");
    if (m_ResourceTable.find(view.GetName()) == m_ResourceTable.end())
    {
		GraphicCore::GetDevice()->CreateShaderResourceView(desc, view);
        m_ResourceTable.emplace(view.GetName());
    }
}

void ResourceContext::CreateConstantBufferView(const RHIConstantBufferViewDesc& desc, RHIConstantBufferViewHandle& view)
{
    AssertGraphics(view.GetName() != L"", "View name invalid - Resource context require unique names for view table");
    if (m_ResourceTable.find(view.GetName()) == m_ResourceTable.end())
    {
		GraphicCore::GetDevice()->CreateConstantBufferView(desc, view);
        m_ResourceTable.emplace(view.GetName());
    }
}

void ResourceContext::CreateUnorderedAccessView(const RHIUnorderedAccessViewDesc& desc, RHIUnorderedAccessViewHandle& view)
{
    AssertGraphics(view.GetName() != L"", "View name invalid - Resource context require unique names for view table");
    if (m_ResourceTable.find(view.GetName()) == m_ResourceTable.end())
    {
		GraphicCore::GetDevice()->CreateUnorderedAccessView(desc, view);
        m_ResourceTable.emplace(view.GetName());
    }
}

ETH_NAMESPACE_END

