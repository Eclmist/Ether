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

