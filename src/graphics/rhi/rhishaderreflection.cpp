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

#include "graphics/graphiccore.h"
#include "graphics/rhi/rhishaderreflection.h"

std::vector<Ether::Graphics::RhiShaderReflection::ResourceBinding> Ether::Graphics::RhiShaderReflection::MergeBindings(
    const std::vector<const RhiShaderReflection*>& reflections)
{
    std::vector<RhiShaderReflection::ResourceBinding> mergedBindings;
    std::unordered_map<std::string, size_t> nameToIndex;

    // Merge bindings from all shaders
    for (const RhiShaderReflection* reflection : reflections)
    {
        for (const auto& binding : reflection->GetResourceBindings())
        {
            // Create unique identifier for this binding
            std::string key = binding.m_Name + "_" + std::to_string(binding.m_BindPoint) + "_" +
                              std::to_string(binding.m_Space);

            if (nameToIndex.contains(key))
                continue;

            RhiShaderReflection::ResourceBinding mergedBinding;
            mergedBinding.m_Name = binding.m_Name;
            mergedBinding.m_Type = binding.m_Type;
            mergedBinding.m_Dimension = binding.m_Dimension;
            mergedBinding.m_BindPoint = binding.m_BindPoint;
            mergedBinding.m_Space = binding.m_Space;

            nameToIndex[key] = mergedBindings.size();
            mergedBindings.push_back(mergedBinding);
        }
    }

    // Sort bindings by type, then by register (for consistent layout)
    std::stable_sort(
        mergedBindings.begin(),
        mergedBindings.end(),
        [](const RhiShaderReflection::ResourceBinding& a, const RhiShaderReflection::ResourceBinding& b)
        {
            if (a.m_Type != b.m_Type)
                return a.m_Type < b.m_Type;
            if (a.m_Space != b.m_Space)
                return a.m_Space < b.m_Space;
            return a.m_BindPoint < b.m_BindPoint;
        });

    return mergedBindings;
}

const Ether::Graphics::RhiShaderReflection::ResourceBinding* Ether::Graphics::RhiShaderReflection::FindBinding(
    const std::string& name) const
{
    if (!m_NameToBindingIndex.contains(name))
        return nullptr;

    return &m_ResourceBindings[m_NameToBindingIndex.at(name)];
}

uint32_t Ether::Graphics::RhiShaderReflection::GetNumConstantBuffers() const
{
    return std::count_if(
        m_ResourceBindings.begin(),
        m_ResourceBindings.end(),
        [](const ResourceBinding& b) { return b.m_Type == RhiDescriptorType::Cbv; });
}

uint32_t Ether::Graphics::RhiShaderReflection::GetNumShaderResources() const
{
    return std::count_if(
        m_ResourceBindings.begin(),
        m_ResourceBindings.end(),
        [](const ResourceBinding& b) { return b.m_Type == RhiDescriptorType::Srv; });
}

uint32_t Ether::Graphics::RhiShaderReflection::GetNumUnorderedAccesses() const
{
    return std::count_if(
        m_ResourceBindings.begin(),
        m_ResourceBindings.end(),
        [](const ResourceBinding& b) { return b.m_Type == RhiDescriptorType::Uav; });
}

