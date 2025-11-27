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

namespace Ether::Graphics
{

class RhiShaderReflection
{
public:
    struct ResourceBinding
    {
        std::string m_Name;
        RhiDescriptorType m_Type;
        RhiResourceDimension m_Dimension;
        uint32_t m_BindPoint; // Register number (the '0' in 't0')
        uint32_t m_BindCount; // Array size (1 for non-arrays)
        uint32_t m_Space;     // Register space

        bool RequiresResourceTable() const
        {
            if (m_Type == RhiDescriptorType::Uav)
                return true; // UAVs always need tables

            if (m_Type == RhiDescriptorType::Cbv)
                return false; // CBVs can be root descriptors

            // For SRVs, check dimension
            switch (m_Dimension)
            {
            case RhiResourceDimension::Buffer:
            case RhiResourceDimension::StructuredBuffer:
            case RhiResourceDimension::RTAccelerationStructure:
                return false; // Use root descriptors for buffers
            case RhiResourceDimension::Texture1D:
            case RhiResourceDimension::Texture1DArray:
            case RhiResourceDimension::Texture2D:
            case RhiResourceDimension::Texture2DArray:
            case RhiResourceDimension::Texture3D:
            case RhiResourceDimension::TextureCube:
            case RhiResourceDimension::TextureCubeArray:
                return true; // Use tables for textures
            default:
                return true; // Default to table
            }
        }
    };

public:
    RhiShaderReflection() = default;
    virtual ~RhiShaderReflection() {};

public:
    virtual void Reflect(const void* shaderBytecode, size_t bytecodeSize, RhiShaderType shaderType) = 0;

public:
    inline const std::vector<ResourceBinding>& GetResourceBindings() const { return m_ResourceBindings; }
    inline const RhiShaderType GetShaderType() const { return m_ShaderType; }

public:
    static std::vector<RhiShaderReflection::ResourceBinding> MergeBindings(const std::vector<const RhiShaderReflection*>& reflections);

public:
    const ResourceBinding* FindBinding(const std::string& name) const;
    uint32_t GetNumConstantBuffers() const;
    uint32_t GetNumShaderResources() const;
    uint32_t GetNumUnorderedAccesses() const;

protected:
    std::vector<ResourceBinding> m_ResourceBindings;
    std::unordered_map<std::string, size_t> m_NameToBindingIndex;
    RhiShaderType m_ShaderType;
};

} // namespace Ether::Graphics
