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

    virtual ~RhiRootSignatureDesc() {}

public:
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
};

class RhiRootSignature
{
public:
    RhiRootSignature() = default;
    virtual ~RhiRootSignature() {}
};

} // namespace Ether::Graphics
