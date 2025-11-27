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

#pragma once

#include "graphics/pch.h"
#include "graphics/rhi/rhishaderreflection.h"

namespace Ether::Graphics
{
class RhiRootSignatureBindingTable
{
public:
    RhiRootSignatureBindingTable(const std::vector<const RhiShaderReflection*>& reflections, RhiPipelineType type);
    ~RhiRootSignatureBindingTable() = default;

public:
    inline bool HasBinding(const std::string& name) const { return m_NameToBinding.contains(name); }

public:
    template <typename T>
    void Bind(GraphicContext& ctx, ResourceContext& rc, const std::string& name, const GFX_STATIC::StaticResourceWrapper<T>& wrapper, uint64_t offset = 0) const;

private:
    void Bind(GraphicContext& ctx, ResourceContext& rc, const std::string& name, RhiShaderVisibleResourceView* resource, uint64_t offset = 0) const;

private:
    struct BindingInfo
    {
        uint32_t m_RootParameterIndex;
        RhiShaderReflection::ResourceBinding m_Binding;
    };

private:
    std::unordered_map<std::string, BindingInfo> m_NameToBinding;
    RhiPipelineType m_PipelineType;
};

template <typename T>
void Ether::Graphics::RhiRootSignatureBindingTable::Bind(
    GraphicContext& ctx,
    ResourceContext& rc,
    const std::string& name,
    const GFX_STATIC::StaticResourceWrapper<T>& wrapper,
    uint64_t offset) const
{
    Bind(ctx, rc, name, wrapper.Get().get(), offset);
}

} // namespace Ether::Graphics
