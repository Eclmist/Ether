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
#include "graphics/rhi/rhirootsignature.h"

namespace Ether::Graphics
{
class RhiComputePipelineStateDesc
{
public:
    RhiComputePipelineStateDesc() = default;
    virtual ~RhiComputePipelineStateDesc() {}

public:
    virtual void SetRootSignature(const RhiRootSignature& rootSignature) = 0;
    virtual void SetComputeShader(const RhiShader& cs) = 0;
    virtual void SetNodeMask(uint32_t mask) = 0;
    virtual void Reset() = 0;

public:
    std::unique_ptr<RhiComputePipelineState> Compile(const char* name) const;
    bool RequiresShaderCompilation() const;
    void CompileShaders();

protected:
    std::unordered_map<RhiShaderType, const RhiShader*> m_Shaders;
};

class RhiComputePipelineState
{
public:
    RhiComputePipelineState(const RhiComputePipelineStateDesc& desc)
        : m_CreationDesc(desc)
    {
    }
    virtual ~RhiComputePipelineState() = default;

protected:
    const RhiComputePipelineStateDesc& m_CreationDesc;
};
} // namespace Ether::Graphics
