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

void Ether::Graphics::RhiShaderReflection::Serialize(OStream& ostream) const
{
    ostream << m_ShaderType;
    ostream << (uint32_t)m_ResourceBindings.size();
    for (const ResourceBinding& binding : m_ResourceBindings)
    {
        ostream << binding.m_Name;
        ostream << binding.m_Type;
        ostream << binding.m_Dimension;
        ostream << binding.m_BindPoint;
        ostream << binding.m_BindCount;
        ostream << binding.m_Space;
        ostream << binding.m_Size;
    }
}

void Ether::Graphics::RhiShaderReflection::Deserialize(IStream& istream)
{
    istream >> m_ShaderType;
    uint32_t numBindings;
    istream >> numBindings; m_ResourceBindings.resize(numBindings);

    for (uint32_t i = 0; i < numBindings; ++i)
    {
        istream >> m_ResourceBindings[i].m_Name;
        istream >> m_ResourceBindings[i].m_Type;
        istream >> m_ResourceBindings[i].m_Dimension;
        istream >> m_ResourceBindings[i].m_BindPoint;
        istream >> m_ResourceBindings[i].m_BindCount;
        istream >> m_ResourceBindings[i].m_Space;
        istream >> m_ResourceBindings[i].m_Size;

        m_NameToBindingIndex[m_ResourceBindings[i].m_Name] = i;
    }
}

std::vector<Ether::Graphics::RhiShaderReflection::ResourceBinding> Ether::Graphics::RhiShaderReflection::MergeBindings(
    const std::vector<const RhiShaderReflection*>& reflections)
{
    std::vector<RhiShaderReflection::ResourceBinding> mergedBindings;
    std::unordered_map<std::string, size_t> nameToIndex;

    // Merge bindings from all shaders
    for (const RhiShaderReflection* reflection : reflections)
    {
        for (const auto& binding : reflection->GetBindings())
        {
            // Create unique identifier for this binding
            std::string key = binding.m_Name + "_" + std::to_string(binding.m_BindPoint) + "_" +
                              std::to_string(binding.m_Space);

            if (nameToIndex.contains(key))
                continue;

            nameToIndex[key] = mergedBindings.size();
            mergedBindings.push_back(binding);
        }
    }

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

