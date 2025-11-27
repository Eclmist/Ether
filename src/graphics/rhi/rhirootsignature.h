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
class RhiRootSignatureDesc
{
public:
    RhiRootSignatureDesc(uint32_t numParams, uint32_t numSamplers)
        : m_NumParameters(numParams)
        , m_NumSamplers(numSamplers)
    {
    }

    virtual ~RhiRootSignatureDesc() = default;

public:
    // NEW and shiny root signature builder from shader reflection!
    virtual void BuildFromReflection(const RhiShaderReflection& reflection) = 0;
    virtual void BuildFromReflection(const std::vector<const RhiShaderReflection*>& reflections) = 0;

public:
    // TODO: Deprecate
    virtual void SetAsConstant(
        uint32_t rootParamterIndex,
        uint32_t shaderRegister,
        uint32_t numDword,
        RhiShaderVisibility visibility) = 0;
    virtual void SetAsConstantBufferView(
        uint32_t rootParamterIndex,
        uint32_t shaderRegister,
        RhiShaderVisibility visibility) = 0;
    virtual void SetAsShaderResourceView(
        uint32_t rootParamterIndex,
        uint32_t shaderRegister,
        RhiShaderVisibility visibility) = 0;
    virtual void SetAsUnorderedAccessView(
        uint32_t rootParamterIndex,
        uint32_t shaderRegister,
        RhiShaderVisibility visibility) = 0;
    virtual void SetAsDescriptorTable(
        uint32_t rootParamterIndex,
        uint32_t numRanges,
        RhiShaderVisibility visibility) = 0;
    virtual void SetDescriptorTableRange(
        uint32_t rootParamterIndex,
        RhiDescriptorType rangeType,
        uint32_t numDescriptors = 1,
        uint32_t rangeIndex = 0,
        uint32_t baseShaderRegister = 0) = 0;
    virtual void SetAsSampler(
        uint32_t shaderRegister,
        RhiSamplerParameterDesc samplerDesc,
        RhiShaderVisibility visibility) = 0;

    virtual void SetFlags(RhiRootSignatureFlag flag) = 0;

public:
    std::unique_ptr<RhiRootSignature> Compile(const char* name) const;

protected:
    uint32_t m_NumParameters;
    uint32_t m_NumSamplers;

    std::vector<RhiShaderReflection::ResourceBinding> m_ShaderBindings;
};

class RhiRootSignature
{
public:
    RhiRootSignature() = default;
    virtual ~RhiRootSignature() {}

public:
    inline const std::string& GetName() const { return m_Name; }
    inline const std::vector<RhiShaderReflection::ResourceBinding>& GetShaderBindings() const { return m_ShaderBindings; }

    inline void SetName(const std::string& name) { m_Name = name; }
    inline void SetShaderBindings(const std::vector<RhiShaderReflection::ResourceBinding>& bindings) { m_ShaderBindings = bindings; }

protected:
    std::string m_Name;
    std::vector<RhiShaderReflection::ResourceBinding> m_ShaderBindings;
};

} // namespace Ether::Graphics
