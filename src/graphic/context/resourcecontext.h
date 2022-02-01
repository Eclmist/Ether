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

#pragma once

#include "graphic/rhi/rhiresource.h"

ETH_NAMESPACE_BEGIN

class ResourceContext : public NonCopyable
{
public:
    ResourceContext(CommandContext& context, const std::wstring& name);
    ~ResourceContext() = default;

    bool CreateBufferResource(uint32_t size, RHIResourceHandle& resource);
    bool CreateTexture2DResource(uint32_t width, uint32_t height, RHIFormat format, RHIResourceHandle& resource);
    bool CreateDepthStencilResource(uint32_t width, uint32_t height, RHIFormat format, RHIResourceHandle& resource);

    bool CreateRenderTargetView(RHIResourceHandle resource, RHIRenderTargetViewHandle& view);
    bool CreateDepthStencilView(RHIResourceHandle resource, RHIDepthStencilViewHandle& view);
    bool CreateShaderResourceView(RHIResourceHandle resource, RHIShaderResourceViewHandle& view);
    bool CreateConstantBufferView(RHIResourceHandle resource, RHIConstantBufferViewHandle& view);
    bool CreateUnorderedAccessView(RHIResourceHandle resource, RHIUnorderedAccessViewHandle& view);

    bool InitializeTexture2D(CompiledTexture& texture);

    void Reset();

private:
    bool CreateResource(const RHICommitedResourceDesc& desc, RHIResourceHandle& resource);
    bool ResourceExists(const std::wstring& resourceID) const;
    bool ShouldRecreateResource(const std::wstring& resourceID, const RHICommitedResourceDesc& desc) const;
    bool ShouldRecreateView(const std::wstring& resourceID) const;

    RHIFormat GetResourceFormat(const std::wstring& resourceID) const;

private:
    std::set<std::wstring> m_ResourceEntries;
    std::set<std::wstring> m_NewlyCreatedResources;
    std::unordered_map<std::wstring, RHICommitedResourceDesc> m_ResourceTable;

    CommandContext* m_Context;
};

ETH_NAMESPACE_END

