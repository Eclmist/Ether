/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2023 Samuel Huang - All rights reserved.

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
#include "graphics/memory/descriptorallocation.h"

namespace Ether::Graphics
{
class BindlessDescriptorManager
{
public:
    BindlessDescriptorManager() = default;
    ~BindlessDescriptorManager() = default;

public:
    static constexpr uint32_t InvalidIndex = -1;

public:
    uint32_t RegisterAsShaderResourceView(StringID resourceGuid, const RhiResource& resource, RhiFormat format);
    uint32_t RegisterAsShaderResourceView(StringID resourceGuid, const RhiResource& resource, RhiVertexBufferViewDesc vb);
    uint32_t RegisterAsShaderResourceView(StringID resourceGuid, const RhiResource& resource, RhiIndexBufferViewDesc ib);
    uint32_t RegisterSampler(StringID name, RhiSamplerParameterDesc& sampler);

    uint32_t GetDescriptorIndex(StringID guid) const;

private:
    std::unordered_map<StringID, uint32_t> m_GuidToIndexMap;
    std::unordered_map<StringID, std::unique_ptr<MemoryAllocation>> m_Allocations;
};
} // namespace Ether::Graphics
