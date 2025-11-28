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
class CommandContext;
class ResourceContext;

class RhiRootSignatureBindingTable
{
public:
    RhiRootSignatureBindingTable() = default;
    ~RhiRootSignatureBindingTable() = default;

public:
    inline bool HasBinding(const std::string& name) const { return m_NameToBinding.contains(name); }
    inline void SetPipelineType(RhiPipelineType type) { m_PipelineType = type; }
    inline void SetResourceContext(const ResourceContext& rc) { m_ResourceContext = &rc; }

#if _DEBUG
    inline void SetDebugName(const std::string& name) { m_DebugName = name; }
    void LogInvalidBinding(const std::string& name) const;
#endif

public:
    void PopulateBindings(const RhiRootSignature& rootSignature);
    
public:
    void Bind(CommandContext& ctx, const std::string& name, RhiShaderVisibleResourceView* resource, uint64_t offset = 0) const;
    void Bind(CommandContext& ctx, const std::string& name, RhiGpuAddress address, uint64_t offset = 0) const;
    void Bind(CommandContext& ctx, const std::string& name, uint32_t value, uint64_t offset = 0) const;

private:
    struct BindingInfo
    {
        uint32_t m_RootParameterIndex;
        RhiShaderReflection::ResourceBinding m_Binding;
    };

private:
    const ResourceContext* m_ResourceContext;
    std::unordered_map<std::string, BindingInfo> m_NameToBinding;
    RhiPipelineType m_PipelineType;

#if _DEBUG
    static std::unordered_set<std::string> s_InvalidBindings;
    std::string m_DebugName;
#endif
};
} // namespace Ether::Graphics

