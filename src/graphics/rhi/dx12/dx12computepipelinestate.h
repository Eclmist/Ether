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
#include "graphics/rhi/rhicomputepipelinestate.h"
#include "graphics/rhi/dx12/dx12includes.h"
#include "graphics/rhi/dx12/dx12pipelinestate.h"

namespace Ether::Graphics
{
class Dx12ComputePipelineStateDesc : public RhiComputePipelineStateDesc
{
public:
    Dx12ComputePipelineStateDesc();
    ~Dx12ComputePipelineStateDesc() override {}

public:
    void SetRootSignature(const RhiRootSignature& rootSignature) override;
    void SetComputeShader(const RhiShader& cs) override;
    void SetNodeMask(uint32_t mask) override;
    void Reset() override;

protected:
    friend class Dx12Device;
    D3D12_COMPUTE_PIPELINE_STATE_DESC m_Dx12PsoDesc;
};

class Dx12ComputePipelineState : public Dx12PipelineState
{
public:
    Dx12ComputePipelineState(const RhiPipelineStateDesc& desc) : Dx12PipelineState(desc) {}
    ~Dx12ComputePipelineState() override = default;
};
} // namespace Ether::Graphics
